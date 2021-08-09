/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */


/* i8259_init(void)
 * Inputs: void
 * Outputs: none
 * Description: initializes master PIC and slave PIC
 *              slave PIC is on master IRQ2
*/
void i8259_init(void) {

    // Initialize Mask Values
    master_mask = 0xFF;
    slave_mask  = 0xFF;

    // Initialize master PIC
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_PIC_DATA);
    outb(ICW3_MASTER, MASTER_PIC_DATA);
    outb(ICW4, MASTER_PIC_DATA);

    // Initialize Slave PIC
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_PIC_DATA);
    outb(ICW3_SLAVE, SLAVE_PIC_DATA);
    outb(ICW4, SLAVE_PIC_DATA);

    //write the IRQ on Master and IRQ on slave to their Port values
    outb(master_mask, MASTER_PIC_DATA);
    outb(slave_mask, SLAVE_PIC_DATA);

    // Connect the IRQ2 line from the Master PIC to the slave PIC
    enable_irq(SLAVE_MASTER_CONNECT);

}

/* enable_irq(uint32_t irq_num)
 * Inputs: irq_num - irq line to enable
 * Outputs: none
 * Description:  Enable (unmask) the specified IRQ
*/
void enable_irq(uint32_t irq_num) {
    uint16_t port;

    // Enables the IRQ for master if it can fit on the Master's PIC ports
    if(irq_num < MAX_PIC_PORTS){
        port = MASTER_PIC_DATA;
        master_mask = master_mask & ~(1 << irq_num);
        outb(master_mask, port);
    }
    // cannot fit on Master PIC ports, so enable the IRQ on the slave
    else{
        port = SLAVE_PIC_DATA;
        irq_num = irq_num - MAX_PIC_PORTS;
        slave_mask = slave_mask & ~(1 << irq_num);
        outb(slave_mask, port);
    }
}

/* enable_irq(uint32_t irq_num)
 * Inputs: irq_num - irq line to enable
 * Outputs: none
 * Description:  Disable (mask) the specified IRQ
*/
void disable_irq(uint32_t irq_num) {
    uint16_t port;

    // Disables the IRQ for master if it can fit on the Master's PIC ports
    if(irq_num < MAX_PIC_PORTS){
        port = MASTER_PIC_DATA;
        master_mask = master_mask | (1 << irq_num);
        outb(master_mask, port);
    }
    // outside of Master PIC ports, so disable the IRQ on the slave
    else{
        port = SLAVE_PIC_DATA;
        irq_num = irq_num - MAX_PIC_PORTS;
        slave_mask = slave_mask | (1 << irq_num);
        outb(slave_mask, port);
    }
}

/* send_eoi(uint32_t irq_num)
 * Inputs: irq_num - irq line to send eoi to
 * Outputs: none
 * Description:  Send end-of-interrupt signal for IRQ input
*/
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {

    // if the IRQ came from the Slave PIC, send max val EOI thru Master and the remaining bits through to Slave
    if(irq_num >= MAX_PIC_PORTS){
        outb(EOI | 2, MASTER_8259_PORT);
        outb(EOI | (irq_num - MAX_PIC_PORTS), SLAVE_8259_PORT);
    }
    // if the IRQ just came from the Master PIC, only need to send it to Master
    else{
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
   
}
