#ifndef __drvcfg_h__
#define __drvcfg_h__

ssize_t pcilat_config_read(struct device *, struct device_attribute *, char *);
ssize_t pcilat_config_write(struct device *, struct device_attribute *, const char *, size_t);

#endif