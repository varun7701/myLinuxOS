#include "rtc.h"

// create two local variables to be used in RTC */
uint8_t prev;
volatile int interrupt_flag;

/* init_rtc()
 * Input: none
 * Output: none
 * Description: turn on periodic RTC interrupt
 *              at irq line 8
 *              https://wiki.osdev.org/RTC
*/
void init_rtc(){

    outb(STATUS_B, RTC_PORT);  // select register B, disable NMI
    prev = inb(RTC_DATA);  // read current val of reg B
    outb(STATUS_B, RTC_PORT);  // set index again to reg B
    outb(prev | 0x40, RTC_DATA);  // mask prev with 0x40, turns on bit 6 of Reg B
     
    enable_irq(rtc_irq);
    interrupt_flag = 0;

}

/* rtc_handler()
 * Input: none
 * Output: none
 * Description: disables all other interrupts, handles RTC interrupt
*/
void rtc_handler(){
    cli();
    outb(STATUS_C, RTC_PORT);
    interrupt_flag = 1;
    inb(RTC_DATA);
    send_eoi(rtc_irq);
    sti();

    
}

/* open_rtc()
 * Input: filename - file name
 * Output: 0
 * Description: initializes RTC frequency to 2HZ, return 0
*/
int32_t open_rtc(const uint8_t* filename){
    set_freq_rtc(2);
    return 0;
}

/* close_rtc()
 * Input: none
 * Output: 0
 * Description:return 0 unless virtualize RTC
*/
int32_t close_rtc(int32_t fd){
    return 0;
}

/* write_rtc()
 * Input: fd- file directory
 *      buf- buffer for desired frequency
 *      nbytes- number of bytes to write
 * Output: 0 on success  or -1 on failure
 * Description: change freq
*/

int32_t write_rtc(int32_t fd, const void* buf, int32_t nbytes){

    cli();
    int frequency = *(uint32_t*)(buf); //desired frequency
    //note may need to switch checks
    if(frequency > 1024 || frequency < 2 || ((frequency & (frequency - 1)) != 0)){ //boundary checks and power of 2 check on frequency
        sti();
        return -1;
    }
    else if( nbytes == 4 && (uint32_t)buf != NULL){

        // check if frequency was able to be set or if it gave an error
        int test = set_freq_rtc(frequency);
        if (test == -1){
            sti();
            return -1;
        }
        else{
            sti();
            return 0; //when setting freq was a success return 0
         }
     } 

    //failed if not trying to write 4 bytes or null desired frequency
    else{
        sti();
        return -1;
    }
}
/* read_rtc()
 * Input: none
 * Output: 0
 * Description:  block until next interrupt
*/
int32_t read_rtc(int32_t fd, void* buf, int32_t nbytes){

    sti();
    set_freq_rtc(512);
    
   while (!interrupt_flag) {    
   }

    interrupt_flag = 0;

    return 0;
}


/* set_freq_rtc()
 * Input: frequency - desired rtc interrupt frequency
 * Output: none
 * Description: set new rtc frequency on port
 */
int32_t set_freq_rtc(int32_t frequency){
    char rate;

    switch(frequency) {
        // sqw output frequency found from register A's select bits  
        //page 13 data sheet Table 5
            case 2:
                rate = 0x0F;
                break;
            case 4:
                rate = 0x0E;
                break;
            case 8:
                rate = 0x0D;
                break;
             case 16:
                rate = 0x0C;
                break;
             case 32:
                rate = 0x0B;
                break;
             case 64:
                rate = 0x0A;
                break;
             case 128:
                rate = 0x09;
                break;
             case 256:
                rate = 0x08;
                break;
             case 512:
                rate = 0x07;
                break;
            case 1024:
                rate = 0x06;
                break;
            default:
                return -1; //return -1 on fail
        }
        cli();
        //get value of regA to rewrite to higher bits
        outb(STATUS_A, RTC_PORT);  //selects reg A as port
        unsigned char regA_temp;
        unsigned char regA_saved = regA_temp & regA_mask;
        //set new rate in register A and restore higher bits
        outb(regA_saved | rate, RTC_DATA); 
        sti();
        return 0;

}
