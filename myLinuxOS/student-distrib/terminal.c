#include "terminal.h"
#include "scheduler.h"
#include "lib.h"
#include "paging.h"
#include "syscall.h"
#include "types.h"


int curr_active_terminal = 0;            // sets our active terminal on boot to Terminal 0
int count = 0;

terminal_t term_blk[MAX_TERMINALS];          // creates our array of three terminals




/* init_terminal()
 * Inputs: none
 * Outputs: none
 * Side Effects: boots our base shell for terminal 1
 * Description: initializes all elements of of our terminal struct, and starts base shell
 *              for the Terminal 1
*/
void init_terminal(){
    int i,j;
    for(i = 0; i < MAX_TERMINALS ; i++){

        if(i == 0)
            term_blk[i].process_active = 0;            // sets the initial terminal to already active
        else
            term_blk[i].process_active = -1;         // which process is active (-1 = nothing, 0 = shell, etc)

        term_blk[i].cursor_x = 0;                    // saves our x location for each terminal
        term_blk[i].cursor_y = 0;                   // saves our y location for each terminal
        term_blk[i].enter_flag = 0;                 // term read stops when enter is hit, and that is because enter = \n

        term_blk[i].video_mem = (uint8_t*)(PAGE_SIZE*((ADDR_VIDEO/BYTES_TO_ALIGN_TO)+i+1));      // should create three 4KB pages right after video mem

        for(j = 0; j < BUFFER_SIZE; j++){                   //clear the key buffers for scheduling 
            term_blk[i].key_buffer[j] = '\0';
        }

    }    

    clear();
    syscall_execute((uint8_t*) "shell");            // start terminal 0 on the boot up
}

/* terminal_open()
 * Inputs: none
 * Outputs: int32_t
 * Side Effects:
 * Description: opens terminal, return 0
*/
int32_t terminal_open(){
  //  clear();
    update_cursor();
    return 0;
}

/* terminal_close()
 * Inputs: none
 * Outputs: int32_t 
 * Description: closes terminal, return 0
*/
int32_t terminal_close(){
    return 0;
}


/* terminal_write()
 * Inputs: fd - file directory(unused)
 *         buf - pointer to buffer with data to write
 *         nbytes - number of bytes to write
 * Outputs: 0 on success, -1 on failure
 * Side Effects: prints to screen
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
   if(buf == NULL){
       return -1;
   }

   int i;

   // skips the null characters and prints the text characters to screen
   for(i= 0; i < nbytes; i++){
       if( ((uint8_t*)buf)[i] == '\0'){
           continue;
       } 
       
       putc(((uint8_t*)buf)[i]);
   }

   return 0;
   //update_cursor();
}

/* terminal_read()
 * Inputs: fd - file directory
 *         buf - pointer to buffer with data to write to
 *         nbytes - number of bytes to write
 * Outputs: number of bytes written
 * Side Effects: copies values in keyboard_buffer to buf
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    
    // set our interrupt flag
    sti();

    uint32_t i;
    uint32_t counter = 0;
    while(enter_flag == 0);
    enter_flag = 0;

   //if more than 128 values are trying to be read, bring it down to 128
   if(nbytes > 128){
       nbytes = 128;
   }

    // if we hit a newline char then we break, else we load buf with our key_buffer values
    for(i = 0; i < nbytes; i++){
        if(key_buffer[i] == '\n'){
            break;
        }
        else{
            ((uint8_t*)buf)[i] = key_buffer[i];
            counter++;
        }

    }
    ((uint8_t*)buf)[i] = '\n';
    counter++;

    //clear keyboard buffer for next read, and return the num of chars put into buf
    buffer_index = -1;
    clear_keyboard_buffer();
    return counter;
}


/* terminal_swap(int new_term_val)
 * Inputs: new_term_val - the new terminal value we are switching to
 * Outputs: 0 if success
 *          -1 if fails
 * Side Effects: swaps the terminal and saves old terminal into vid mem and loads new term into visible vid mem
*/
int32_t terminal_swap(int new_term_val){

    cli();
    int retval = -1;
    
    // validity check
    if(new_term_val < 0 || new_term_val >= MAX_TERMINALS)
        return -1;

    // do nothing if we try to change to already active term
    if(new_term_val == curr_active_terminal){
        return 0;
    }

    // if terminal is never opened yet, we save current terminal, restore new terminal vidmem, run shell on it
    if(term_blk[new_term_val].process_active == -1){
        
        retval = save_visible_terminal(curr_active_terminal);
        retval = restore_new_terminal(new_term_val);

        curr_active_terminal = new_term_val;

        syscall_execute((uint8_t*) "shell");

        sti();
        return 0; 

    }    

    // the terminal already existed, so we do not rerun shell, just save current and restore new terminal
    retval = save_visible_terminal(curr_active_terminal);
    retval = restore_new_terminal(new_term_val);
    curr_active_terminal = new_term_val;

    sti();
    return 0; 
    
}


/* save_visible_terminal(int terminal_old)
 * Inputs: terminal_old - the current terminal we are saving into its backup vidmem page
 * Outputs: 0 if success
 * Side Effects: copy the current video on screen into its terminal backup vidmem page, saves cursor_x and cursor_y
*/
int32_t save_visible_terminal(int terminal_old){

    memcpy(term_blk[terminal_old].video_mem, (uint8_t*)ADDR_VIDEO, PAGE_SIZE); // save whats in curr video mem into its backup 
    term_blk[terminal_old].cursor_x = retrieve_screen_x();
    term_blk[terminal_old].cursor_y = retrieve_screen_y();

    clear_keyboard_buffer();
    return 0;

}

/* restore_new_terminal(int terminal_new)
 * Inputs: terminal_new - the terminal we are switching to
 * Outputs: 0 if success
 * Side Effects: assign cursor x and y, copy from the terminal_new's backup vidmem page onto screen video
*/
int32_t restore_new_terminal(int terminal_new){

    // Frustrating way to set screen_x and screen_y
    assign_screen_x(term_blk[terminal_new].cursor_x);
    assign_screen_y(term_blk[terminal_new].cursor_y);
    update_cursor();
    memcpy((uint8_t*)ADDR_VIDEO, term_blk[terminal_new].video_mem, PAGE_SIZE);

    return 0;

}


