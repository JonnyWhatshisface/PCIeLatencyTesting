#ifndef __iomem_h__
#define __iomem_h__

ssize_t pcilat_iomem_read(struct device *, struct device_attribute *, char *);
ssize_t pcilat_iomem_write(struct device *, struct device_attribute *, const char *, size_t);
#endif