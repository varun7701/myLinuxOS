 #include "paging.h"
 #include "lib.h"
 #include "types.h"
 #include "x86_desc.h"
 #include "test_enable.h"
 #include "syscall.h"

 #define ASM 1


/* initialize_paging
 *
 * Creates the paging directory and tables
 * Inputs: None
 * Outputs: none
 * Side Effects: creates page directory and table
 * Files: paging.c/h, test_enable.S/h
*/
 void initialize_paging(){

   unsigned int i;

   for(i=0; i < MAX_PAGE_ENTRIES; i++){

     if(i == 0){     // this is going to be used for our Page table

       page_directory[i].pde_present = 1;         // sets the page table to present
       page_directory[i].pde_user = 0;
       page_directory[i].pde_size = 0;
       page_directory[i].pde_addr = (uint32_t)(page_table) / BYTES_TO_ALIGN_TO;       // dividing by BYTES_TO_ALIGN_TO is the same as shifting our address 12 bits since this is unsigned ints
     }
     else if (i == 1){     // this is where the kernel starts

        page_directory[i].pde_present = 1;         // sets the page table to present
        page_directory[i].pde_user = 0;
        page_directory[i].pde_size = 1;
        page_directory[i].pde_addr = KERNEL_LOCATION / BYTES_TO_ALIGN_TO;
     }
     else if (i == USER_PROGRAM_INDEX){      // the user program starts at 128MB/4MB

       page_directory[i].pde_present = 1;         // sets the page table to present
       page_directory[i].pde_user = 1;
       page_directory[i].pde_size = 1;
       page_directory[i].pde_addr =  USER_LOCATION / BYTES_TO_ALIGN_TO;
     }
     else{

       page_directory[i].pde_present = 0;         // since present and user are set in the other locations, we need to give an else function that sets to 0 otherwise they will get overwritten
       page_directory[i].pde_user = 0;
       page_directory[i].pde_size = 1;          // everything after the kernel space should be organized in 4MB sections up until 4GB
     }


     page_directory[i].pde_rw = 1;
     page_directory[i].pde_wt = 0;
     page_directory[i].pde_dis_cache = 0;         // initialize the rest of the directory to R/W = 1, and unused elements to 0.
     page_directory[i].pde_accessed = 0;
     page_directory[i].pde_reserved = 0;
     page_directory[i].pde_avalaible = 0;

   }


   for(i = 0; i < MAX_PAGE_ENTRIES; i++){

        page_table[i].pte_present = 0;
        page_table[i].pte_rw = 1;
        page_table[i].pte_user = 0;
        page_table[i].pte_wt = 0;
        page_table[i].pte_dis_cache = 0;                 // setup the page table to be r/w and their addresses to be equal to our loop variable
        page_table[i].pte_accessed = 0;
        page_table[i].pte_dirty = 0;
        page_table[i].pte_reserved = 0;
        page_table[i].pte_global = 0;
        page_table[i].pte_avalaible = 0;
        page_table[i].pte_addr = i;

   }

        page_table[ADDR_VIDEO / BYTES_TO_ALIGN_TO].pte_present = 1;                                   // initialize the video address space by setting present = 1
        page_table[ADDR_VIDEO / BYTES_TO_ALIGN_TO].pte_addr = ADDR_VIDEO / BYTES_TO_ALIGN_TO;         // set the address of the page table location equal to the video address / 1024
        
        
        page_table[(ADDR_VIDEO / BYTES_TO_ALIGN_TO) + 1].pte_present = 1;  
        page_table[(ADDR_VIDEO / BYTES_TO_ALIGN_TO) + 2].pte_present = 1;         // set the three addresses after the video memory (B8000) to present for terminals
        page_table[(ADDR_VIDEO / BYTES_TO_ALIGN_TO) + 3].pte_present = 1;  
        

       uint32_t pass_in = (uint32_t)(page_directory);           // create pointer to page dir, pass into test_enable to enable paging

       test_enable(pass_in);



  // video map table initialize
  for(i = 0; i < MAX_PAGE_ENTRIES; i++){

    page_vidmap[i].pte_present = 0;
    page_vidmap[i].pte_rw = 1;
    page_vidmap[i].pte_user = 0;
    page_vidmap[i].pte_wt = 0;
    page_vidmap[i].pte_dis_cache = 0;                 // setup and init the page table for our vidmap
    page_vidmap[i].pte_accessed = 0;                  
    page_vidmap[i].pte_dirty = 0;
    page_vidmap[i].pte_reserved = 0;
    page_vidmap[i].pte_global = 0;
    page_vidmap[i].pte_avalaible = 0;
    page_vidmap[i].pte_addr = i;

  }

}

/* syscall_helper
 *
 * Restores the parent paging 
 * Inputs: pid - parent pid 
 * Outputs: none
 * Side Effects: updates the page directory to the parent's address
 */
void syscall_helper(uint32_t pid){

       page_directory[USER_PROGRAM_INDEX].pde_present = 1;         // sets the page table to present
       page_directory[USER_PROGRAM_INDEX].pde_user = 1;
       page_directory[USER_PROGRAM_INDEX].pde_size = 1;
       page_directory[USER_PROGRAM_INDEX].pde_addr =  (pid*PAGE_SIZE_4MB + (KPAGE_SIZE))/BYTES_TO_ALIGN_TO;

}

/* syscall_vidmap_helper
 *
 * helper function to map video memory in user space
 * Inputs: none 
 * Outputs: none
 * Side Effects: maps the vidmap table to our PDE and sets the first entry in PTE to video mem address
 */
void syscall_vidmap_helper(){

       page_directory[VIDMAP_INDEX].pde_present = 1;         // sets the page table to present
       page_directory[VIDMAP_INDEX].pde_user = 1;
       page_directory[VIDMAP_INDEX].pde_size = 0;
       page_directory[VIDMAP_INDEX].pde_addr =  (uint32_t)(page_vidmap)/BYTES_TO_ALIGN_TO;

       page_vidmap[0].pte_present = 1;
       page_vidmap[0].pte_user = 1;
       page_vidmap[0].pte_addr = ADDR_VIDEO / BYTES_TO_ALIGN_TO;


}
