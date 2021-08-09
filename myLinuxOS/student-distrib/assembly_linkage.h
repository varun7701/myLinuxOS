#ifndef _ASSEMBLY_LINKAGE_H
#define _ASSEMBLY_LINKAGE_H

#include "rtc.h"
#include "keyboard.h"
#include "syscall.h"

/* Interrupt ASM Linkage */
extern void KEY_INTERRUPT();
extern void RTC_INTERRUPT();
extern void SYSCALL_ASM();


#endif
