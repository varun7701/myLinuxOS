#ifndef _SCHEDULER_H
#define _SCHEDULER_H

/* I/O ports https://wiki.osdev.org/Programmable_Interval_Timer */
#define CH0 0x40
#define CH1 0x41
#define CH2 0x42

#define mode_reg 0x43
#define Mode3 0x36 /* generate a square wave Mode 3 http://www.osdever.net/bkerndev/Docs/pit.htm */

#define IRQ0  0x00
#define HZ_42 28409/* can be 1193180/42Hz= 28409*/




// extern void init_scheduler(void); 
// extern void scheduler_enable(void);
// uint8_t terminal_curr;

#endif
