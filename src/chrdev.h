#ifndef __chrdev_h__
#define __chrdev_h__

int pcilat_dev_open(struct inode *, struct file *);
ssize_t pcilat_dev_read(struct file *, char __user *, size_t , loff_t *);
char *pci_char_devnode(struct device *, umode_t *);

#endif
