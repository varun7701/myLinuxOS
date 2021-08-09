#define ASM 1

.text


.globl KEY_INTERRUPT
.globl RTC_INTERRUPT
.globl SYSCALL_ASM


/* RTC_INTERRUPT
 *
 * used for interrupts for rtc. assembly linkage. calls rtc_handler
 * Inputs: None
 * Outputs: none
*/
RTC_INTERRUPT:

    pushal
    call rtc_handler
    popal
    iret

/* KEY_INTERRUPT
 *
 * used for interrupts for keyboard. assembly linkage. calls keyboard_interrupt
 * Inputs: None
 * Outputs: none
*/
KEY_INTERRUPT:

    pushal
    /* pushfl */
    call keyboard_interrupt
    /* popfl */
    popal

    iret



/* SYSCALL_ASM
 *
 * used for system calls execution
 * Inputs: eax - system call 
 * Outputs: none
 * Side Effects: uses jumpe table to use the appropriate system call function
*/
SYSCALL_ASM:

    /* check for call-code between 0 and 9, inclusive */
    cmpl $0, %eax
    jle INVALID_CALL
    cmpl $10, %eax
    jg INVALID_CALL

    /* push arguments 
    According to 
    https://en.wikibooks.org/wiki/X86_Assembly/Interfacing_with_Linux#System_calls
     */

    pushl %esp
    pushl %ebp
    pushl %edi
    pushl %esi
    pushfl
    pushl %edx
    pushl %ecx
    pushl %ebx
 

    call *jump_table-4(,%eax,4)

    /* teardown */
    popl %ebx
    popl %ecx
    popl %edx
    popfl
    popl %esi
    popl %edi
    popl %ebp
    popl %esp

    iret


INVALID_CALL:
    movl $-1, %eax
    iret


jump_table:
    .long syscall_halt, syscall_execute, syscall_read, syscall_write, syscall_open, syscall_close, syscall_getargs, syscall_vidmap, syscall_set_handler, syscall_sigreturn


.end










.end
