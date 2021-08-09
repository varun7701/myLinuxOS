#ifndef _TEST_ENABLE_H
#define _TEST_ENABLE_H

#include "types.h"

/* Enable paging and Flush TLB functions */
extern void test_enable(uint32_t a);
extern void flush_tlb(void);

#endif
