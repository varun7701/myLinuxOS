#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "i8259.h"
#include "tests.h"

#define RTC_PORT 0x70   // select register port
#define RTC_DATA 0x71   // read/write port
#define rtc_irq  8      // rtc interrupt occupies irq line 8
#define regA_mask 0xF0  // bitmask value to get four bits

#define STATUS_A 0x8A
#define STATUS_B 0x8B   // Status Registers
#define STATUS_C 0x8C

/* initialized RTC interrupt line on PIC */
void init_rtc();

/* disable all other interrupts, handle RTC interrupt */
void rtc_handler();

/* RTC Driver Operations */
int32_t open_rtc(const uint8_t* filename);
int32_t close_rtc(int32_t fd);
int32_t write_rtc(int32_t fd, const void* buf, int32_t nbytes);
int32_t read_rtc(int32_t fd, void* buf, int32_t nbytes);
int32_t set_freq_rtc(int32_t frequency);

#endif
