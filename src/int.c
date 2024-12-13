#include "driver.h"
#include "int.h"

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