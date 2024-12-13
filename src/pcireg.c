#include "driver.h"
#include "pcireg.h"

ssize_t pcilat_devreg_read(struct device *dev, struct device_attribute *attr, char *buf) {
    struct pcilat_priv *priv = dev_get_drvdata(dev);
    char *out = "This device allows reading or setting specific registers\non the PCI device.\n\nTo use, you'll need to write out the specific\nregister you want to read. To set\na register, write the register and value.\n\nExample (R): echo \"0x10\" >pcilat_devreg\nExample (W): echo \"0x10 1\" >pcilat_devreg\n";
    dev_info(dev, "Opened devreg read device\n");

    return scnprintf(buf, PAGE_SIZE, out);
}

ssize_t pcilat_devreg_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    struct pcilat_priv *priv = dev_get_drvdata(dev);
    int fields, rc, tmp;
    char arg1[200], arg2[200];
    char *check = buf; /* Yes, we discard the quantifier here. It's fine. No, really... */
    check[strlen(check)-1] = '\0';
    fields = sscanf(check, "%s %s", arg1, arg2);

    if (strcmp(arg2, "") == 0) {
        // read register
        dev_info(dev, "Register read operation\n");
    } else {
        // write register
        dev_info(dev, "Register write operation\n");
    }

    return count;
}

int register_test(struct device *dev, const char *buf) {
    struct pcilat_priv *priv = dev_get_drvdata(dev);
    struct pci_dev *pdev = priv->pdev;

    u32 stat_dword;
    u16 stat_word;
    u8 stat_byte;

        /*

    Messing with registers....

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
    // Prints 255...
    pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &byte);
    dev_info(&pdev->dev, "Val: %d\n", byte);
    */

    return 0;

}