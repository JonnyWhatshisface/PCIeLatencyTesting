#include "driver.h"
#include "chrdev.h"

int pcilat_dev_open(struct inode *inode, struct file *file) {
    /* Setup private structure and make it available to all fops */
	struct pcilat_priv *priv = container_of(inode->i_cdev, struct pcilat_priv, cdev);
	file->private_data = priv;

    return 0;
}

ssize_t pcilat_dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    /* And now we can access pcilat_priv like so */
    struct pcilat_priv *priv = file->private_data;

    return 0;
}

char *pci_char_devnode(struct device *dev, umode_t *mode) {
    struct pci_dev *pdev = to_pci_dev(dev->parent);
    return kasprintf(GFP_KERNEL, DRIVER_NAME "/%02x:%02x.%x", pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
}
