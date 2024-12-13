#include "driver.h"
#include "tsc.h"

static char device[2048] __initdata;

static int pcilat_probe(struct pci_dev *, const struct pci_device_id *);
static void pcilat_remove(struct pci_dev *);

static struct pci_driver pcilat_driver = {
    .name       =   DRIVER_NAME,
    .id_table   =   NULL, /* We use dynamic ID's */
    .probe      =   pcilat_probe,
    .remove     =   pcilat_remove
};

module_param_string(device, device, sizeof(device), 0);
MODULE_PARM_DESC(device, "Initial PCI IDs to add to the driver. Command separated "
                "list formatted as \"vendor:device[:subvendor[:subdevice[:class[:class_mask]]]]\""
                );

/*
    Initialize sysfs related bits. These will be created
    per device. For example, if the device id is 02:00.0
    then you will find the devices under:

    /sys/bus/pci/devices/0000:02:00.0/pcilatdriver/02:00.0/pcilat_*
*/

static ssize_t pcilat_config_read(struct device *dev, struct device_attribute *attr, char *buf) {
    /* Show configuration options */
    struct pcilat_priv *priv = dev_get_drvdata(dev);
    char *out = "bar %d\nloops %d\nndelay %d\n";

    return scnprintf(buf, PAGE_SIZE, out, priv->test.bar, priv->test.loops, priv->test.ndelay);
}

static ssize_t pcilat_config_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    /* 
        Set configuration options for the benchmark tests.
        Invalid settings return -EINVAL.
    */
    struct pcilat_priv *priv = dev_get_drvdata(dev);
    int fields, rc, tmp;
    char arg1[200], arg2[200];
    char *check = buf; /* Yes, we discard the quantifier here. It's fine. No, really... */
    check[strlen(check)-1] = '\0';
    fields = sscanf(check, "%s %s", arg1, arg2);

    if (strcmp(arg1, "bar") == 0) {
            rc = kstrtoint(arg2, 0, &tmp);
            if (tmp >= 0 && tmp < 6 && priv->bar[tmp].addr != NULL) {
                priv->test.bar = tmp;
                dev_info(&priv->pdev->dev, "Config Set: BAR %d - Done\n", tmp);
            } else {
                dev_info(&priv->pdev->dev, "Config Set: BAR %d - Failed: not a valid bar\n", tmp);                
                return -EINVAL;
            }
    } else if (strcmp(arg1, "loops") == 0) {
            rc = kstrtoint(arg2, 0, &tmp);
            if (tmp > 0 && tmp <= 500000) {
                priv->test.loops = tmp;
                dev_info(&priv->pdev->dev, "Config Set: LOOPS %d - Done\n", tmp);
            } else {
                dev_info(&priv->pdev->dev, "Config Set: LOOPS %d - Failed: not a valid value [1-500000]\n", tmp);
                return -EINVAL;
            }
    } else if (strcmp(arg1, "ndelay") == 0) {
            rc = kstrtoint(arg2, 0, &tmp);
            if (tmp > 0 && tmp <= 50000) {
                priv->test.ndelay = tmp;
                dev_info(&priv->pdev->dev, "Config Set: NDELAY %d - Done\n", tmp);
            } else {
                dev_info(&priv->pdev->dev, "Config Set: LOOPS %d - Failed: not a valid value [1-50000]\n", tmp);
                return -EINVAL;
            }
    }

    return count;
}

static ssize_t pcilat_iomem_read(struct device *dev, struct device_attribute *attr, char *buf) {
    dev_info(dev, "Opened iomem measure device\n");
    return 0;
}

static ssize_t pcilat_iomem_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    /* 
        Takes options to trigger a latency measurement to device. Format should be:
    
        CPUS BAR ITERATIONS or just CPUS

        CPU's can be a list with ranges, i.e. 0,1,3-4,6

        To read results of all latency measurements, simply cat this device.

     */
    return 0;
}

static ssize_t pcilat_interrupt_read(struct device *dev, struct device_attribute *attr, char *buf) {
    dev_info(dev, "Opened interrupt device\n");
    return 0;
}

static ssize_t pcilat_interrupt_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    /* 
        Used to trigger and measure latency of interrupt raising and servicing.
    
        To read the results stored, simply cat this device.
     */
    struct pcilat_priv *priv = dev_get_drvdata(dev);
    int fields, rc, irq, cpu, intnum, i, found;
    char arg1[200], arg2[200];
    char *check = buf; /* Yes, we discard the quantifier here. It's fine. No, really... */
    check[strlen(check)-1] = '\0';
    fields = sscanf(check, "%s %s", arg1, arg2);
    rc = kstrtoint(arg1, 0, &irq);
    rc = kstrtoint(arg2, 0, &cpu);
    found = 0;

    if(irq < 0) {
        dev_info(&priv->pdev->dev, "Trigger Interrupt: IRQ %d invalid...\n", irq);
        goto done;
    }
    
    /* Iterate our registered IRQ's and see if this is one of them */
    for(i = 0; i < 32; i++)
        if (priv->irqs[i] == irq)
            found = 1;
    if (!found) {
        /* Not ours so not our business... */
        dev_info(&priv->pdev->dev, "IRQ %d does not belong to this device...\n", irq);
        goto done;
    }

    /* IRQ belongs to this device */
    dev_info(&priv->pdev->dev, "%d belongs to us, calling interrupt trigger\n", irq);
    trigger_interrupt(irq);
    
done:    
    return count;
}

static DEVICE_ATTR(pcilat_config, 0644, pcilat_config_read, pcilat_config_write);
static DEVICE_ATTR(pcilat_iomem, 0644, pcilat_iomem_read, pcilat_iomem_write);
static DEVICE_ATTR(pcilat_interrupt, 0644, pcilat_interrupt_read, pcilat_interrupt_write);

static struct attribute *pcilat_attrs[] = {
    &dev_attr_pcilat_config.attr,
    &dev_attr_pcilat_iomem.attr,
    &dev_attr_pcilat_interrupt.attr,
    NULL,
};

ATTRIBUTE_GROUPS(pcilat);

/*
    Just a fun fact... The ATTRIBUTE_GROUPS(pcilat) macro actually expands to:

        static struct attribute_group pcilat_group = {
            .attrs = pcilat_attrs,
        };

        static const struct attribute_group *pcilat_groups[] = {
            &pcilat_group,
            NULL,
        };

*/

static int pcilat_dev_open(struct inode *inode, struct file *file) {
    /* Setup private structure and make it available to all fops */
	struct pcilat_priv *priv = container_of(inode->i_cdev, struct pcilat_priv, cdev);
	file->private_data = priv;

    return 0;
}

static ssize_t pcilat_dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    /* And now we can access pcilat_priv like so */
    struct pcilat_priv *priv = file->private_data;

    return 0;
}

static const struct file_operations pcilat_chr_ops = {
    .owner      =   THIS_MODULE,
    .open       =   pcilat_dev_open,
    .read       =   pcilat_dev_read
};

static char *pci_char_devnode(struct device *dev, umode_t *mode) {
    struct pci_dev *pdev = to_pci_dev(dev->parent);
    return kasprintf(GFP_KERNEL, DRIVER_NAME "/%02x:%02x.%x", pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
}

static int pcilat_probe(struct pci_dev *pdev, const struct pci_device_id *id) {
    /*
        During the probe, we'll map desired BAR's and registry IRQ handlers
    */
    int err, i, mem_bars, ret, irq, ff = 0;
    struct pcilat_priv *priv;
    struct device *dev;

    u16 statreg;

    pr_info(DRIVER_NAME ": Probing device %02x:%02x.%x", pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
    priv = kzalloc(sizeof(struct pcilat_priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;
    /* Assign test defaults */
    priv->test.ndelay = 800;
    priv->test.loops = 1000;
    priv->test.type = TEST_RO;
    priv->pdev = pdev;

    /* We can use pci_enable_device_mem() to only enable IORESOURCE_MEM BARs,
       but ultimately we do want to also test IO performance so let's just go
       ahead and enable all BARs now
    */
    err = pci_enable_device(pdev);
    if (err)
        goto fail_pci_regions;

    /* For the moment being we do not care about IORESOURCE_IO BARs */
    dev_info(&pdev->dev, "Scanning for BARs with IORESOURCE_MEM only...\n");
    mem_bars = pci_select_bars(pdev, IORESOURCE_MEM);
    err = pci_request_selected_regions(pdev, mem_bars, DRIVER_NAME);
    if (err)
        goto fail_pci_regions;

    for (i = 0; i < 6; i++) {
        if(mem_bars & (1 << i)) {
            dev_info(&pdev->dev, "Found usable BAR: %d\n", i);
            priv->bar[i].addr = ioremap(pci_resource_start(pdev, i), pci_resource_len(pdev, i));
            if (!ff) {
                priv->test.bar = i;
                ff = 1;
            }
            if (IS_ERR(priv->bar[i].addr)) {
                err = PTR_ERR(priv->bar[i].addr);
                dev_info(&pdev->dev, "Failed ioremap() for BAR %d\n", i);
                break;
            }
        } else {
            priv->bar[i].addr = NULL;
            priv->bar[i].len = -1;
        }
    }

    if (err)
        goto fail_ioremap_or_chrdev_region;

    err = alloc_chrdev_region(&priv->dev_num, 0, 1, DRIVER_NAME);
    if(err)
        goto fail_ioremap_or_chrdev_region;

    cdev_init(&priv->cdev, &pcilat_chr_ops);
    priv->cdev.owner = THIS_MODULE;

    err = cdev_add(&priv->cdev, priv->dev_num, 1);
    if (err)
        goto fail_cdev_add;

    dev = device_create(pcilat_class, &pdev->dev, priv->dev_num, NULL, "%02x:%02x.%x", pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));

    if (IS_ERR(dev)) {
        err = PTR_ERR(dev);
        goto fail_cdev_create;
    }

    dev_set_drvdata(dev, priv);
    pci_set_drvdata(pdev, priv);

    // Register MSI or MSIx IRQs...
    dev_info(&pdev->dev, "Requesting MSI[x] IRQ Vectors, min: %d max: %d\n", MSI_IRQ_MIN, MSI_IRQ_MAX);
    ret = pci_alloc_irq_vectors(pdev, MSI_IRQ_MIN, MSI_IRQ_MAX, PCI_IRQ_MSI | PCI_IRQ_MSIX);
    if (ret < 0)
        goto fail_alloc_irq_vectors;
    priv->irq_vectors = ret;
    dev_info(&pdev->dev, "Received %d vectors\n", ret);
    
    for (i = 0; i < ret; i++) {
        int rc;
        irq = pci_irq_vector(pdev, i);
        dev_info(&pdev->dev, "Registering vector %d: IRQ %d\n", i, irq);
        priv->irqs[i] = irq;
        rc = request_threaded_irq(irq, irq_handler, NULL, 0, DRIVER_NAME, (void *) irq_handler);
    }
    for (; i <= 32; i++) {
        priv->irqs[i] = -1;
    }
    pci_intx(pdev, 1);
    pci_read_config_word(pdev, PCI_COMMAND, &statreg);
    if((statreg >>10) & 0x1) {
        dev_info(&pdev->dev, "Status is 1\n");
    } else {
        dev_info(&pdev->dev, "Status is 0\n");
    }

    
    statreg |= 0 >>0;
    pci_write_config_word(pdev, PCI_COMMAND, PCI_COMMAND & ~(1 <<0x10));
    pci_read_config_word(pdev, PCI_COMMAND, &statreg);
    if((statreg >>10) & 0x1) {
        dev_info(&pdev->dev, "Status 1\n");
    } else {
        dev_info(&pdev->dev, "Status 0\n");
    }

    dev_info(&pdev->dev, "Device now claimed by " DRIVER_NAME "\n");

    return 0;

fail_alloc_irq_vectors:
    err = -1;

fail_cdev_create:
    cdev_del(&priv->cdev);

fail_cdev_add:
    unregister_chrdev_region(priv->dev_num, 0);

fail_ioremap_or_chrdev_region:
    for (i=0; i < 6; i++)
        if (priv->bar[i].addr != NULL)
            iounmap(priv->bar[i].addr);

    pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));

fail_pci_regions:
    pci_disable_device(pdev);
    kfree(priv);

    return err;
}

static int __init pcilat_init(void) {
    int err;
    char *p, *devid;

    pr_info(DRIVER_NAME ": Driver loaded, initializing... CPU: %d\n", get_cpu());

    pcilat_class = class_create(THIS_MODULE, DRIVER_NAME);
    if (IS_ERR(pcilat_class)) {
        err = PTR_ERR(pcilat_class);
        goto load_fail;
    }

    pcilat_class->devnode = pci_char_devnode;
    pcilat_class->dev_groups = pcilat_groups;

    err = pci_register_driver(&pcilat_driver);
    if (err) {
        // Destroy class and error
        class_destroy(pcilat_class);
        goto load_fail;
    }

    if (device[0] == '\0') { 
        /* Driver was loaded with no device specified */
        pr_info(DRIVER_NAME ": No device specified. You can do so by adding the vendor/product and binding a device like so:\n");
        pr_info(DRIVER_NAME ":     echo \"<vendor_id> <product_id>\" > /sys/bus/pci/drivers/pcilatency/new_id\n");
        pr_info(DRIVER_NAME ":     echo <pci_bus_id> > /sys/class/pci/drivers/pcilatency/bind\n");
        goto load_finished;
    } else {
        /* Driver was loaded with device(s) specified */
        p = device;
        pr_info(DRIVER_NAME ": Driver initialized with device ID(s) specified: %s\n", p);
        while ((devid = strsep(&p, ","))) {
            unsigned int vendor, device, subvendor = PCI_ANY_ID,
                         subdevice = PCI_ANY_ID, class = 0, class_mask = 0;
            int fields;
        

            if (!strlen(devid))
                continue;

            fields = sscanf(devid, "%x:%x:%x:%x:%x:%x", &vendor, &device, &subvendor, &subdevice, &class, &class_mask);
            if (fields < 2) {
                pr_warn(DRIVER_NAME ": Invalid device ID string \"%s\"\n", devid);
                continue;
            }

            pr_info(DRIVER_NAME ": Adding %04X:%04X sub=%04X:%04X cls=%08X/%08X\n", vendor, device, subvendor, subdevice, class, class_mask);
            err = pci_add_dynid(&pcilat_driver, vendor, device, subvendor, subdevice, class, class_mask, 0);
            if (err)
                pr_warn(DRIVER_NAME ": Unable to add device id %d\n", err);
        }
        /* Finished scanning and checking device ID's */
        goto load_finished;
    }

load_finished:
    return 0;

load_fail:
    return err;
}

static void pcilat_remove(struct pci_dev *pdev) {
    int i, irq;
    struct pcilat_priv *priv = pci_get_drvdata(pdev);

    dev_info(&pdev->dev, "Removing driver...\n");

    /* First we free up our IRQs */
    for (i = 0; i < priv->irq_vectors; i++) {
        irq = pci_irq_vector(pdev, i);
        dev_info(&pdev->dev, "Freeing vector %d: IRQ %d\n", i, irq);
        free_irq(irq, (void *) irq_handler);
    }
    pci_free_irq_vectors(pdev);
    /* Remove any mappings for the BARs */
	for (i = 0; i < 6; i++) {
		if (priv->bar[i].addr != NULL) {
            dev_info(&pdev->dev, "Unmapping memory for BAR %d\n", i);
			iounmap(priv->bar[i].addr);
        }
    }
    /* Destroy device and classes, unregister and free data */
    device_destroy(pcilat_class, priv->dev_num);
    cdev_del(&priv->cdev);
    unregister_chrdev_region(priv->dev_num, 0);
    pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
    pci_disable_device(pdev);
    kfree(priv);
    /* Last bit happens in __exit routine */
}

static void __exit pcilat_exit(void) {
    pci_unregister_driver(&pcilat_driver);
    class_destroy(pcilat_class);
    pr_info(DRIVER_NAME ": Unloaded PCI Latency Testing Driver\n");
}

module_init(pcilat_init);
module_exit(pcilat_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Device Interrupt and Access Latency Tester");
MODULE_AUTHOR("Jonathan D. Hall <Jon@JonathanDavidHall.com>");
