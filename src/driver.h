#ifndef __driver_h__
#define __driver_h__

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/err.h>

#define DRIVER_NAME     "pcilatdriver"
#define MSI_IRQ_MIN     4                   /* Minimum number of vectors / IRQs */
#define MSI_IRQ_MAX     4                   /* Maximum number of vectors / IRQs */


static struct class *pcilat_class;

enum test_type {
    TEST_RO,
    TEST_WO,
    TEST_RW
};

struct test_config {
    int bar;
    int loops;
    int ndelay;
    enum test_type type;
};

struct bar_t {
	int len;
	void __iomem *addr;
};

struct pcilat_priv {
    struct pci_dev *pdev;
    struct bar_t bar[6];
    struct test_config test;
    dev_t dev_num;
    struct cdev cdev;
    int irq_vectors;
    int irqs[32];
};

irqreturn_t irq_handler(int,void *);
int trigger_interrupt(int);
#endif