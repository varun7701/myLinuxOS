#ifndef _PAGING_H
#define _PAGING_H

#ifndef ASM

#include "lib.h"
#include "types.h"


#define MAX_PAGE_ENTRIES 1024           // 2^10 PTES per table
#define PAGE_SIZE 4096                  // 2^12 bytes per PAGE_SIZE
#define BYTES_TO_ALIGN_TO 4096          // 4kB


#define PAGE_SIZE_4MB   0x400000          // 4MB

#define ADDR_VIDEO 0xB8000              // video address start is 0xB8000
#define KERNEL_LOCATION 0x400000        // kernel starts in memory at 4MB
#define USER_LOCATION 0x8000000         // user program starts in memory at 128MB

#define USER_PROGRAM_INDEX  32          // the 128 MB is the starting location and since each entry is 4MB in the PDT we know it starts at 32
#define VIDMAP_INDEX 34                 // the 136 MB is the starting location and each entry is 4MB, so 136/4 = 34 

/* Page Directory Entry */
typedef struct __attribute__ ((packed)) page_directory_entry {
    uint32_t pde_present    :1;
    uint32_t pde_rw         :1;
    uint32_t pde_user       :1;
    uint32_t pde_wt         :1;
    uint32_t pde_dis_cache  :1;
    uint32_t pde_accessed   :1;
    uint32_t pde_reserved   :1;
    uint32_t pde_size       :1;
    uint32_t pde_global     :1;
    uint32_t pde_avalaible  :3;
    uint32_t pde_addr       :20;
} page_directory_entry;


/* Page Table Entry */
typedef struct __attribute__ ((packed)) page_table_entry {
    uint32_t pte_present    :1;
    uint32_t pte_rw         :1;
    uint32_t pte_user       :1;
    uint32_t pte_wt         :1;
    uint32_t pte_dis_cache  :1;
    uint32_t pte_accessed   :1;
    uint32_t pte_dirty      :1;
    uint32_t pte_reserved   :1;
    uint32_t pte_global     :1;
    uint32_t pte_avalaible  :3;
    uint32_t pte_addr       :20;
} page_table_entry;

/* Create our two Page Tables and Page Directory */
page_table_entry page_table[MAX_PAGE_ENTRIES] __attribute__ ((aligned (BYTES_TO_ALIGN_TO)));
page_directory_entry page_directory[MAX_PAGE_ENTRIES] __attribute__ ((aligned (BYTES_TO_ALIGN_TO)));
page_table_entry page_vidmap[MAX_PAGE_ENTRIES] __attribute__ ((aligned (BYTES_TO_ALIGN_TO)));


/* initialize the page directory and tables */
extern void initialize_paging();

/* helper function to restore paging to the parent pid */
extern void syscall_helper(uint32_t pid);

/* helper to map video memory into user space */
extern void syscall_vidmap_helper();

#endif
#endif
