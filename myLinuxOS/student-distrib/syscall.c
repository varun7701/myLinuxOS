#include "syscall.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "filesys.h"
#include "paging.h"
#include "x86_desc.h"
#include "test_enable.h"


// array to see which PID is occupied
uint8_t pid_array[MAX_PID] = {0, 0, 0, 0, 0, 0};

// initialize our pid and parent pid
uint32_t pid = 0;
uint32_t parent_pid = 0;


// entry instruction pointer and stack location for user program
uint32_t entry_point;
uint32_t entry_stack_location;

// set our global exception flag to 0
int globalflag = 0;

/* syscall_halt
 * Input: status - 
 * Outputs:  0 - returns to exec process
 *          -1 - if fails
 * Description: halts the process, return value to parent
*/
int32_t syscall_halt(uint8_t status){

pcb_t* curr_pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));

    /* close files */
    int i;
    for(i = 0; i < 8; i++){
        curr_pcb->fd_array[i].flags = 0;
    }

    if(curr_pcb->pid == 0){

        pid = PID_HALT_VAL;
        pid_array[0] = 0;

        int ret_check = syscall_execute((uint8_t*) "shell");


        if(ret_check != PID_HALT_VAL){

        asm volatile (

            "pushl %%ebx        ; "
            "pushl %%edx        ; "
            "pushfl             ; "
            "popl  %%ebx        ; "
            "orl $0x200, %%ebx  ; "
            "pushl %%ebx        ; "
            "pushl %%ecx        ; " 
            "pushl %%eax        ; "    
            "iret               ; "

            :
            : "a" (entry_point), "b" (USER_DS), "c" (USER_CS), "d" (entry_stack_location)
            : "memory"

        );

        }
    }

    /* setup file state */
    pid_array[pid] = 0;
    pid = curr_pcb->parent_pid;
    pcb_t* parent_pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));
    parent_pid = parent_pcb->parent_pid;
 

    /* restore parent paging */
    syscall_helper(curr_pcb->parent_pid);
    flush_tlb();

    /* set TSS to parent kernel stack */
    tss.esp0 = KPAGE_SIZE - (curr_pcb->parent_pid)*KSTACK_SIZE - MEM_FENCE;
    tss.ss0 = KERNEL_DS;

    /* swap to parent stack and ret from execute */
    curr_pcb->exception_flag = (uint32_t)(status) && 0xF;

    /* check if exception handler called halt */
    if(status == 0 && globalflag == 1){
        curr_pcb->exception_flag = EXCEPT_HALT;
        // clear global flag after setting exception_flag
        globalflag = 0;
    }

    asm volatile(
        "movl %0, %%esp        ; "
        "movl %1, %%ebp        ; "
        "movl %2, %%eax        ; "
        "jmp return_label      ; "
        :
        : "b" (curr_pcb->execute_esp), "c" (curr_pcb->execute_ebp), "a" (curr_pcb->exception_flag)
        : "memory"
    );

    status = (uint8_t)(curr_pcb->exception_flag);

    /* if it reaches this point it means the halt did not return to execute, so failure */
    return 0;

}


/* syscall_execute
 * Input: command - contains filename and arg
 * Outputs:   256 - dies by exception
 *           0 - if halt system call
 *          -1 - if failure
 * Description: load and execute a new program
*/
int32_t syscall_execute(uint8_t* command){
    
    // needed separate counter for PID otherwise we boot loop
    int pid_loop;

    /* 1. Assign PID */
    for(pid_loop = 0; pid_loop < 6; pid_loop++){
        if(pid_array[pid_loop] == 0){
            pid_array[pid_loop] = 1;
            pid = pid_loop;
            term_blk[curr_active_terminal].process_active++; 
            break;
        }
    }
    if(pid_loop == 6){   //hit process limit
        syscall_write(1, "reached max amount\n", 19);
        return 0;
    }


    /* 2. Allocate PCB */
    pcb_t* curr_pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));
    curr_pcb->pid = pid;

    if(pid == 0){
        curr_pcb->parent_pid = 0;
    }
    else{
        curr_pcb->parent_pid = parent_pid;
        parent_pid = pid;
    }

    //set up file descriptors for stdin and stdout
    curr_pcb->fd_array[0].file_ops_table = &stdin_ops_table;
    curr_pcb->fd_array[0].inode = 0;
    curr_pcb->fd_array[0].flags = 1;
    curr_pcb->fd_array[0].file_pos = START_FILE_POS;

    curr_pcb->fd_array[1].file_ops_table = &stdout_ops_table;
    curr_pcb->fd_array[1].inode = 0;
    curr_pcb->fd_array[1].flags = 1;
    curr_pcb->fd_array[1].file_pos = START_FILE_POS;

    /* 3. Parse Command*/
    //extract executable name from command
    uint8_t exe_name[FILENAME_MAX_LENGTH];
    uint8_t i,k;
    uint32_t counter = 0;
    uint8_t j = 0;

    // fill buffers with null chars 
    for(i = 0; i < FILENAME_MAX_LENGTH; i++){
        exe_name[i] = '\0';
    }

    for(i = 0; i < ARG_LENGTH; i++){
        curr_pcb->args[i] = '\0';
    }

   //gets file name, adds args to pcb after trailing spaces
    for(i = k = 0; i < strlen((int8_t*) command) + 1; i++, k++){

        if(i == 0){
           while(command[i] == ' ')
            i++;
        }

        //everything before the first null, newline, or space is part of executable name
        if((command[i] != ' ') && (command[i] != '\0') && (command[i] != '\n')){
            exe_name[k] = command[i];
        }
        else if(command[i] == ' ' || command[i] == '\0'){
            for(j = i; j < strlen((int8_t*) command); j++){
                if(command[j] != ' '){
                    break;
                }
            }
            break;
        }

        else{
            break;
        }
    }

    // reached the arguments section of command line
    for(k = j; k < strlen((int8_t*) command); k++){
        curr_pcb->args[counter] = command[k];
        counter++;
    }
    

    /* 4. Find File and Check */
    dentry_t exe_dentry;
    uint8_t buffer[MEM_FENCE];
    //check if file exists in file directory
    if(read_dentry_by_name(exe_name, &exe_dentry) != 0){
        pid_array[pid] = 0;
        return -1;
    }
    //read data into a buffer to check for starting characters
    int32_t read_check = read_data(exe_dentry.inode, 0, buffer, MEM_FENCE);
    if(read_check == -1){
        pid_array[pid] = 0;
        return -1;
    }
    //if the first 4 chars aren't Delete, E, L, F, error
    if(buffer[0] != DELETE || buffer[1] != E_CHAR || buffer[2] != L_CHAR || buffer[3] != F_CHAR){
        pid_array[pid] = 0;
        return -1;
    }


    /* 5. Allocate and setup paging */
    syscall_helper(pid);
    flush_tlb();

    /* 6. Copy Program to Allocated Page */
    inode_t* exec_inode_ptr = (inode_t*)(inode_ptr + exe_dentry.inode);
    read_data(exe_dentry.inode, 0, (uint8_t*)IMAGE_ADDRESS, exec_inode_ptr->length);

    /* 7. Setup TSS for context switch */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KPAGE_SIZE - pid*KSTACK_SIZE - MEM_FENCE;

    uint8_t exec_buffer[MEM_FENCE];
    read_data(exe_dentry.inode, EXEC_START, exec_buffer, MEM_FENCE);

    entry_point = *((uint32_t*)exec_buffer);
    entry_stack_location = USER_LOCATION + PAGE_SIZE_4MB - sizeof(int32_t);

    /* 8. Store state of curr. stack */


    asm volatile(

        "movl %%esp, %%eax        ; "
        "movl %%ebp, %%ebx        ; "

        : "=a" (curr_pcb->execute_esp), "=b" (curr_pcb->execute_ebp)    
    );


    /* 9. Perform iret context switch */

    //set the exception flag in pcb to 0, eventually get updated
    curr_pcb->exception_flag = 0;


    asm volatile(
    
        "pushl %%ebx        ; "
        "pushl %%edx        ; "
        "pushfl             ; "
        "popl  %%ebx        ; "
        "orl $0x200, %%ebx  ; "
        "pushl %%ebx        ; "
        "pushl %%ecx        ; " 
        "pushl %%eax        ; "    
        "iret               ; "
        "return_label       :; "
        "leave              ; "
        "ret                ; "

        :
        : "a" (entry_point), "b" (USER_DS), "c" (USER_CS), "d" (entry_stack_location)
        : "memory"

    );

    return 0;

}

/* syscall_read
 * Input: fd- file directory
 *      buf- buffer to write from
 *      nbytes- number of bytes to write
 * Outputs:  returns number of bytes read
 *           0 - if at end of file
 *          -1 - if failure
 * Description: reads data from specific location
*/
int32_t syscall_read(int32_t fd, void* buf, int32_t nbytes){

    // argument validity
    if(buf == NULL || fd < 0 || fd > MAX_NUM_FD){
        return -1;
    }

    
    // create pcb and return file's read value
    pcb_t* pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));
    if(pcb->fd_array[fd].flags == 0){
        return -1;
    }
    return (pcb->fd_array[fd].file_ops_table->read_fn(fd, buf, nbytes));

}

/* syscall_write
 * Input: fd- file directory
 *      buf- buffer to write from
 *      nbytes- number of bytes to write
 * Outputs:  returns the number of bytes to be written
 *          -1 - if failure
 * Description: writes data to the terminal or device
*/
int32_t syscall_write(int32_t fd, void* buf, int32_t nbytes){
    //check argument validity
    if(fd < 0 || nbytes < 0 || fd > MAX_NUM_FD || buf == NULL){
        return -1;
    }

    // sti(); 
    pcb_t* pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));
    //if fd entry is not being used, it is invalid
    if(pcb->fd_array[fd].flags == 0){
        return -1;
    }
    return (pcb->fd_array[fd].file_ops_table->write_fn(fd, buf, nbytes));
}

/* syscall_open
 * Inputs: uint8_t* filename - name of file to be opened
 * Outputs: fd - file descriptor now being used, if successful
 *          -1 - if failure
 * Description: allocate file descriptor array entry to file, call open from driver
*/
int32_t syscall_open(const uint8_t* filename){

    pcb_t* pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));

    int32_t i;
    int32_t fd;

    // argument validity check
    if(filename == NULL || strlen((char*)filename) == 0) 
        return -1;


    dentry_t curr_entry;
    int32_t check_file = read_dentry_by_name(filename, &curr_entry);
    //check if the file name existed
    if(check_file == -1){
        return -1;
    }

    //find usable array entry (when flag != 1)
    for(i = START_FD; i < MAX_NUM_FD; i++){
        if(pcb->fd_array[i].flags==0){
            fd = i;
            pcb->fd_array[i].flags = 1;
            
            //check file type execute knows which specific device handler to call
            if(curr_entry.type == 0){
                pcb->fd_array[fd].file_ops_table = &rtc_ops_table;
            }
            if(curr_entry.type == 1){
                pcb->fd_array[fd].file_ops_table = &dir_ops_table;
            }
            if(curr_entry.type == 2){
                pcb->fd_array[fd].file_ops_table = &file_ops_table;
            }
    
            pcb->fd_array[fd].file_pos = START_FILE_POS;
            pcb->fd_array[fd].inode = curr_entry.inode;
            //as this fd array entry is now being used, set flags to 1
            pcb->fd_array[fd].flags = 1;

            return fd;
        }
    }

    return -1;

}


/* syscall_close
 * Inputs: int32_t fd - file descriptor array entry to close
 * Outputs: -1 if invalid arg
 *          close function for file type if valid arg
 * Description: Frees file descriptor array entry, calls desired close driver
*/
int32_t syscall_close(int32_t fd){
    pcb_t* pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));

    //check invalid descriptors (0 for input, 1 for output)
    if(fd < START_FD || fd > MAX_NUM_FD){
        return -1;
    }

    //set flag back to 0 so descriptor can be used by another file
    if(pcb->fd_array[fd].flags == 1){
        pcb->fd_array[fd].flags = 0;
        pcb->fd_array[fd].inode = 0;
        pcb->fd_array[fd].file_pos = START_FILE_POS;
        return pcb->fd_array[fd].file_ops_table->close_fn(fd);

    }else{
        return -1;
    }


}


/* syscall_getargs
 * Inputs: buf - user-level buffers
 *         nbytes - the number of bytes going to be read
 * Outputs: -1 if no args or if cannot fit into buffer
 *           0 if args read correctly
 * Description: reads the command line args into user buffer
*/
int32_t syscall_getargs(uint8_t* buf, int32_t nbytes){

    int i, j;
    // arg validity check
    if(buf == NULL)
        return -1;
    
    pcb_t* pcb = (pcb_t*)(KPAGE_SIZE - (KSTACK_SIZE * (pid + 1)));

    if(pcb->args[0] == NULL)
        return -1;
    // strip the spaces off the args buffer
    for(i = j = 0; i < nbytes; i++){


        if(pcb->args[i] != ' '){

            buf[j] = pcb->args[i];   
            j++; 

            if(pcb->args[i+1] == ' '){
                buf[j] = ' ';
                j++;
            }
        }


    }

    // add null char to end and ret
    buf[j] = '\0';

    return 0;


}




/* syscall_vidmap
 * Inputs: uint8_t** screen_start - memory location provided by caller
 * Outputs: -1 if invalid arg
 *           0 if video memory sets up correctly
 * Description: maps the video memory into user space at pre-set virtual address
*/
int32_t syscall_vidmap(uint8_t** screen_start){

    // validity check on args
    if(screen_start == NULL)
        return -1;

    // validity check on args
    if(screen_start <= (uint8_t**)(USER_LOCATION) || screen_start > (uint8_t**)(USER_LOCATION + PAGE_SIZE_4MB))
        return -1;

    // call helper function to map video memory to user space
    syscall_vidmap_helper();

    flush_tlb();

    *screen_start = (uint8_t*)(VIRTUAL_VIDMAP);

    return 0;

}


/* syscall_set_handler
 * Inputs: signum - the specific signal handler to change
 *         handler_address - pointer to user function when signal is received
 * Outputs: -1 if invalid arg
 *          close function for file type if valid arg
 * Description: Frees file descriptor array entry, calls desired close driver
*/
int32_t syscall_set_handler(int32_t signum, void* handler_address){
    return -1;
}


/* syscall_sigreturn
 * Inputs: int32_t fd - file descriptor array entry to close
 * Outputs: -1 if invalid arg
 *          close function for file type if valid arg
 * Description: Frees file descriptor array entry, calls desired close driver
*/
int32_t syscall_sigreturn(void){
    return -1;
}



/* ops_table_initialize()
 * Inputs: none
 * Outputs: none
 * Description: Initialize the different ops tables
*/
void ops_table_initialize(){


rtc_ops_table.read_fn = read_rtc;
rtc_ops_table.write_fn = write_rtc;
rtc_ops_table.open_fn = open_rtc;
rtc_ops_table.close_fn = close_rtc;

dir_ops_table.read_fn = dir_read;
dir_ops_table.write_fn = dir_write;
dir_ops_table.open_fn = dir_open;
dir_ops_table.close_fn = dir_close;

file_ops_table.read_fn = file_read;
file_ops_table.write_fn = file_write;
file_ops_table.open_fn = file_open;
file_ops_table.close_fn = file_close;

/* stdin does not have access to write */
stdin_ops_table.read_fn = terminal_read;
stdin_ops_table.write_fn = bad_write;
stdin_ops_table.open_fn = 0;
stdin_ops_table.close_fn = 0;

/* stdout does not have access to read */
stdout_ops_table.read_fn = bad_read;
stdout_ops_table.write_fn = terminal_write;
stdout_ops_table.open_fn = 0;
stdout_ops_table.close_fn = 0;

}

/* bad_read
 * Input:  fd- file directory
 *         buf- buffer to write from
 *         nbytes- number of bytes to write
 * Outputs: none
 * Description: dummy function for stdout
*/
int32_t bad_read (int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

/* bad_write
 * Input:  fd- file directory
 *         buf- buffer to write from
 *         nbytes- number of bytes to write
 * Outputs: none
 * Description: dummy function for stdin
*/
int32_t bad_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}
