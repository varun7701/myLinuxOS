#include "scheduler.h"
#include "terminal.h"


void init_scheduler(void){
    outb(mode_reg, Mode3); /* set mode register to square wave*/
   /* set to interupt every 42 seconds*/
    outb(HZ_42 & 0xFF, CH0);  //lower byte of divisor
    outb(HZ_42 >> 8, CH0); //higher byte of divisor
    /* CH0 output is to the PIC chip*/
    enable_irq(IRQ0);
    return;
}


void scheduler_enable(void){
    /*
    send_eoi(IRQ0);

    //get current process
    pcb_t* curr_pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1))); 
    //save current process's esp and ebp 
    asm volatile(

        "movl %%esp, %%eax        ; "
        "movl %%ebp, %%ebx        ; "

        : "=a" (curr_pcb->execute_esp), "=b" (curr_pcb->execute_ebp)    
    );

    //get next process?
    curr_pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (curr_pcb->parent_pid + 1))); 

    //setup tss for context switch
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KPAGE_SIZE - (curr_pcb->pid)*KSTACK_SIZE - MEM_FENCE;

    //setup paging for next process
    syscall_helper(curr_pcb->pid);

    //asm to move esp/ebp from pcb to actual ebp and esp
    asm volatile(
        
        "movl %0, %%ebp            ; "
        "movl %1, %%esp            ; "

        : "=r" (curr_pcb->execute_ebp), "=r" (curr_pcb->execute_esp)
    );

    flush_tlb();

    */
}
