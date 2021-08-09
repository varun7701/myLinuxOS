#define ASM 1

.globl test_enable
.align 4

/* test_enable
 *
 * used to enable paging by modifying control regs. based on https://wiki.osdev.org/Paging
 * Inputs: None
 * Outputs: none
 * Side Effects: enables pe, pse, sets pdtr
*/
test_enable:

      pushl %ebp              /* stack creation*/
      movl %esp, %ebp
      movl 8(%ebp), %eax      /*get argument for and store into cr3 - page directory pointer */
      movl %eax, %cr3

      movl %cr4, %eax         /*enable pse in cr4 */
      orl $0x00000010, %eax
      movl %eax, %cr4

      movl %cr0, %eax         /*enablepg and pe in cr0 */
      orl $0x80000001, %eax
      movl %eax, %cr0

      leave
      ret

.globl flush_tlb
.align 4


/* flush_tlb
 *
 * used to flush our cr3 register 
 * Inputs: None
 * Outputs: none
 * Side Effects: flushes the cr3 register after we interact with user paging
*/

flush_tlb:

      movl %cr3, %eax
      movl %eax, %cr3
      ret

