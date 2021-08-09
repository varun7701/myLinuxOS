#ifndef _IDT_H
#define _IDT_H
#ifndef ASM


#include "keyboard.h"
#include "rtc.h"
#include "assembly_linkage.h"

/* Interrupt Indices */
#define KEYBOARD_INTERRUPT_INDEX 0x21
#define RTC_INTERRUPT_INDEX 0x28
#define RESERVED 32

/* Signal Values for IDT */
#define HIGH 0x1
#define LOW 0x0
#define USER_LEVEL 0x3

/* System calls execute by excecuting int $0x80 */
#define SYSCALL_INTERRUPT_IDX 0x80

/* IDT init */
void initialize_idt();

#endif
#endif
