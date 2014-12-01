#include <interrupt.h>
#include <reg.h>

#define CLOCK_TO_10_MILLI OSTMR_FREQ / 100

/* IRQ_Handler updates system time whenever OS timer match 0 IRQ is serviced */
void irq_handler() {

    // ensure the irq is that from the match0 and counter0
    if (reg_read(INT_ICPR_ADDR) & (1 << 26)) {
        // update the current time
        current_time += 10;

        //handshake
        reg_set(OSTMR_OSSR_ADDR,OSTMR_OSSR_M0);

        // call again in 10 millis
        unsigned new_time = (unsigned)CLOCK_TO_10_MILLI + reg_read(OSTMR_OSCR_ADDR);
        reg_write(OSTMR_OSMR_ADDR(0), new_time);
    }

}