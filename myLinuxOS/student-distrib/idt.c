#include "exceptions.h"
#include "tests.h"
#include "x86_desc.h"
#include "idt.h"


/* initialize_idt()
 * Inputs: none
 * Outputs: none
 * Description: Initializes the interrupt descriptior table
*/
void initialize_idt(){

    // load the idt table by passing in the ptr to it
    lidt(idt_desc_ptr);
    int i;

    // initialize IDT entries
    for(i = 0; i < NUM_VEC; i++){

        idt[i].seg_selector = KERNEL_CS;    //KERNEL_CS gets loaded when interrupt, exception, or int instruction called
        idt[i].reserved4 = LOW;                
        idt[i].reserved2 = HIGH;        // bits 12 through 8 (excluding 11) of the Offset values for interrupt handler
        idt[i].reserved1 = HIGH;        // bit 11 is determined below for whether hardware or system descriptor
        idt[i].reserved0 = LOW;
        idt[i].size = HIGH;             // indicates 32-bit interrupt gate
        idt[i].present = HIGH;          // indicates valid interrupt descriptor

        // Hardware interrupt and exception descriptors
        if(i < RESERVED){
            idt[i].reserved3 = HIGH;
            idt[i].dpl = LOW;
        }
        // System Call descriptor
        else{
           idt[i].reserved3 = LOW;
           idt[i].dpl = USER_LEVEL;         // user level programs need DPL set to 3
       }
    }

    // connect exceptions to their respective IDT entry
    SET_IDT_ENTRY(idt[0], exception0);
    SET_IDT_ENTRY(idt[1], exception1);
    SET_IDT_ENTRY(idt[2], exception2);
    SET_IDT_ENTRY(idt[3], exception3);
    SET_IDT_ENTRY(idt[4], exception4);
    SET_IDT_ENTRY(idt[5], exception5);
    SET_IDT_ENTRY(idt[6], exception6);
    SET_IDT_ENTRY(idt[7], exception7);
    SET_IDT_ENTRY(idt[8], exception8);
    SET_IDT_ENTRY(idt[9], exception9);
    SET_IDT_ENTRY(idt[10], exception10);
    SET_IDT_ENTRY(idt[11], exception11);
    SET_IDT_ENTRY(idt[12], exception12);
    SET_IDT_ENTRY(idt[13], exception13);
    SET_IDT_ENTRY(idt[14], exception14);
    SET_IDT_ENTRY(idt[15], exception15);
    SET_IDT_ENTRY(idt[16], exception16);
    SET_IDT_ENTRY(idt[17], exception17);
    SET_IDT_ENTRY(idt[18], exception18);
    SET_IDT_ENTRY(idt[19], exception19);

    // connect interrupts
    SET_IDT_ENTRY(idt[KEYBOARD_INTERRUPT_INDEX], KEY_INTERRUPT);
    SET_IDT_ENTRY(idt[RTC_INTERRUPT_INDEX], RTC_INTERRUPT);
    SET_IDT_ENTRY(idt[SYSCALL_INTERRUPT_IDX], SYSCALL_ASM);


}
