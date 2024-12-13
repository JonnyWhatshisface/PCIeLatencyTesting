#ifndef __int_h__
#define __int_h__

#define FIRST_EXTERNAL_VECTOR               0x20
#define IRQ0_VECTOR                         ((FIRST_EXTERNAL_VECTOR + 16)& ~15)

irqreturn_t irq_handler(int,void *);
int trigger_interrupt(int);

ssize_t pcilat_interrupt_read(struct device *, struct device_attribute *, char *);
ssize_t pcilat_interrupt_write(struct device *, struct device_attribute *, const char *, size_t);

#endif
