#include "driver.h"
#include "iomem.h"

ssize_t pcilat_iomem_read(struct device *dev, struct device_attribute *attr, char *buf) {
    dev_info(dev, "Opened iomem measure device\n");
    return 0;
}

ssize_t pcilat_iomem_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    /* 
        Takes options to trigger a latency measurement to device. Format should be:
    
        CPUS BAR ITERATIONS or just CPUS

        CPU's can be a list with ranges, i.e. 0,1,3-4,6

        To read results of all latency measurements, simply cat this device.

     */
    return 0;
}
