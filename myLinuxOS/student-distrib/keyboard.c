#include "keyboard.h"
#include "terminal.h"

// Lowercase ascii values 
static uint8_t ascii_val[ARRAY_SIZE] = {

    'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9',
    '`', '-', '=', '[', ']', '\\',
    ';', '0', ',', '.', '/', ' ',
    

};

// Shift ascii values
static uint8_t shift_ascii[ARRAY_SIZE] = {

    'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X', 
    'Y', 'Z', ')', '!', '@', '#',
    '$', '%', '^', '&', '*', '(',
    '~', '_', '+', '{', '}', '|',
    ':', '"', '<', '>', '?', ' '
};

// Capslock ascii values
static uint8_t caps_ascii[ARRAY_SIZE] = {

    'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X', 
    'Y', 'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9',
    '`', '-', '=', '[', ']', '\\',
    ';', '0', ',', '.', '/', ' ',
};


// Keymapping the scancodes
static uint8_t key_in_array[ARRAY_SIZE] = {

    0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21,     // A,B,C,D,E,F
    0x22, 0x23, 0x17, 0x24, 0x25, 0x26,     // G,H,I,J,K,L
    0x32, 0x31, 0x18, 0x19, 0x10, 0x13,     // M,N,O,P,Q,R
    0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D,     // S,T,U,V,W,X
    0x15, 0x2C, 0x0B, 0x02, 0x03, 0x04,     // Y,Z,0,1,2,3
    0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,      // 4,5,6,7,8,9
    0x29, 0x0C, 0x0D, 0x1A, 0x1B, 0x2B,     // `,-,=,[,],'\'
    0x27, 0x28, 0x33, 0x34, 0x35, 0x39,     // ;,'',',',.,/

};

volatile char key_buffer[EXTENDED_BUFFER];   // buffer used to store chars to be printed
int buffer_index;                            // keeps track of where we are in buffer

// cmd key flags
volatile int32_t shift_flag;
volatile int32_t ctrl_flag;
volatile int32_t capslock_flag;
volatile int32_t enter_flag;
volatile int32_t alt_flag;

/* init_keyboard()
 * Inputs: none
 * Outputs: none
 * Description: initializes keyboard interrupt line on PIC 
*/
void init_keyboard(){
    enable_irq(keyboard_irq);
    buffer_index = -1;
}

/* keyboard_interrupt()
 * Inputs: none
 * Outputs: none
 * Description: hanldes keyboard interrupt, prints val to screen 
*/
void keyboard_interrupt(){
   
    int retval = 0;

    //Begin critical section
    cli();
    unsigned char key_in;
    key_in = inb(KEYBOARD_PORT);    //get scancode from keyboard port
    send_eoi(keyboard_irq);
    sti();

    // setting the CMD key flags based on what is pressed
    if(key_in == ENTER_SCANCODE){
        enter_flag = 1;
    }
    if(key_in == ENTER_RELEASE){
        enter_flag = 0;
    }

   if(key_in == RIGHT_SHIFT || key_in == LEFT_SHIFT){
       shift_flag =1;
   }
   else if(key_in == RIGHT_SHIFT_RELEASE || key_in == LEFT_SHIFT_RELEASE){
       shift_flag = 0;
   }
   else if(key_in == CTRL_PRESS){
       ctrl_flag = 1;
   }
   else if(key_in == CTRL_RELEASE){
       ctrl_flag = 0;
   }
   else if(key_in == CAPS_PRESS){
       if(capslock_flag == 0){
           capslock_flag = 1;
       }
       else{
           capslock_flag = 0;
       }
   }
   else if(key_in == BACKSPACE_SCANCODE){

        if(buffer_index >= 0){
           backspace();
           buffer_index--;          //decrement previous char out of buffer
       }
       
   }

    else if(key_in == ALT_PRESS){
        alt_flag = 1;
   }
   else if(key_in == ALT_RELEASE){
        alt_flag = 0;
   }
   else if(ctrl_flag && key_in == L_SCANCODE){  //ctrl + l/L clears screen
        clear();
   }

   // terminal swap handling based on ALT + F1/F2/F3
   else if (alt_flag && key_in == F1_SCANCODE){
        retval = terminal_swap(0);
   }
   else if (alt_flag && key_in == F2_SCANCODE){
        retval = terminal_swap(1);
   }
   else if (alt_flag && key_in == F3_SCANCODE){
        retval = terminal_swap(2);
   }

    // if nothing else is happening, put the key_in on screen
   else{
       printToScreen(key_in);
   }

    if(retval == -1){
        printf("failure in swap \n");
    }

    //Restore flags, end critical section
    return;

}

/* printToScreen(char key_in)
 * Inputs: key_in - scancode from keyboard
 * Outputs: none
 * Side Effects: prints keystroke to the console
 * Description: gets ascii value of scancode and prints to screen
*/
void printToScreen(char key_in){
    int i,j;

    // makes the TAB key put four spaces instead of one
    if(key_in == TAB_SCANCODE){
        for(j = 0; j < 3; j++){
            buffer_index++;
            key_buffer[buffer_index] = ' ';
            putc(key_buffer[buffer_index]);
        }
        return;
    }
    
    // prints out a newline if enter is pressed
    if(key_in == ENTER_SCANCODE){
        buffer_index++;
        key_buffer[buffer_index] = '\n';
        putc(key_buffer[buffer_index]);
        return;
    }
    
    for(i = 0; i < ARRAY_SIZE; i++){    
        if(key_in == key_in_array[i]){  //if scancode is a valid key
            buffer_index++;
            if(shift_flag && !capslock_flag){   //only shift
                key_buffer[buffer_index] = shift_ascii[i];
                putc(key_buffer[buffer_index]);
            }
            else if(capslock_flag && !shift_flag){  //only capslock
                key_buffer[buffer_index] = caps_ascii[i];
                putc(key_buffer[buffer_index]);
            }  
            else if(capslock_flag && shift_flag){   //shift and caps
                key_buffer[buffer_index] = ascii_val[i];
                putc(key_buffer[buffer_index]);
            }
            else{
                key_buffer[buffer_index] = ascii_val[i];
                putc(key_buffer[buffer_index]);
            }
            
            break;
        }
    }
    return;
}

/* clear_keyboard_buffer()
 * Inputs: none
 * Outputs: none
 * Side Effects: clears the keyboard buffer by putting NULL into each index
 * Description: fills buffer with NULL values
*/
void clear_keyboard_buffer(){
    uint32_t i;
    for(i = 0; i < EXTENDED_BUFFER; i++){
        key_buffer[i] = '\0';                   // set all values in buffer to null chars
    }
}
