#include "driver.h"
#include "int.h"

ssize_t pcilat_interrupt_read(struct device *dev, struct device_attribute *attr, char *buf) {
    dev_info(dev, "Opened interrupt device\n");
    return 0;
}

ssize_t pcilat_interrupt_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
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

irqreturn_t irq_handler(int irq, void *cookie) {
    (void) cookie;
    pr_info(DRIVER_NAME ": Intterrupt entered... Handling IRQ %d on CPU %d\n", irq, get_cpu());

    return IRQ_HANDLED;
}

int trigger_interrupt(int irq) {
    /* Raise interrupt on specified IRQ handler */
    pr_info(DRIVER_NAME ": Entered interrupt trigger\n");

    int val = IRQ0_VECTOR + irq;
    pr_info(DRIVER_NAME ": Triggering interrupt on %d\n", val);

#ifdef __aarch64__
    /* AMD64 does not have an "int" instruction */
    /* WIP and unfinished. Adding this because I'm 
       doing some of the writing via M2 macbook and
       it would just be nice to have.
                                                */

    //asm("smc 0x60");

#elif __x86_64__
    // Example for x86_64
    // We'll need to raise the interrupt
    // by calling int against the IRQ
    // handler.
    //asm("int $0x60");

#endif

return 0;
}