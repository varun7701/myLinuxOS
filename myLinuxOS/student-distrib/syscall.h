#ifndef _SYSCALL_H
#define _SYSCALL_H



#include "lib.h"
#include "types.h"


#define KPAGE_SIZE 0x800000                     // 8MB page
#define KSTACK_SIZE 8192                        // 8KB stack size
#define MEM_FENCE 4                             // same as sizeof(int32)
#define EXCEPT_HALT 256                         // return val for exception 
#define MAX_PID 6                               // max PIDs that can be open
#define PID_HALT_VAL -1                         // pid value when it halts in base

#define EXEC_START 24                           // where the execute will start of the program

#define DELETE 0x7F                             // four magic number defines to check validity
#define E_CHAR 0x45
#define L_CHAR 0x4c
#define F_CHAR 0x46

#define IMAGE_ADDRESS 0x08048000                // the image address    
#define VIRTUAL_VIDMAP 0x8800000                // address for our vidmap page table in user space

#define START_FILE_POS 0                        // base file position for the file system array

#define MAX_NUM_FD  8                           // 8 files can be open at once
#define START_FD    2                           // files 0,1 are taken, so we start checking from 2-7

#define ARG_LENGTH 128                          // The arguments buffer can hold 128

//jump table for file operations
typedef struct ops_table_t{
    int32_t (*read_fn)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*open_fn)(const uint8_t* filename);
    int32_t  (*close_fn)(int32_t fd);
    int32_t (*write_fn)(int32_t fd, const void* buf, int32_t nbytes);
}ops_table_t;

// individual file descriptor entries for fd_array in pcb
typedef struct file_descriptor_t{
    ops_table_t* file_ops_table;
    int32_t inode;
    int32_t file_pos;
    int32_t flags;
}file_descriptor_t;


// PCB structure to hold PIDs and different stack/entry pointers
typedef struct pcb_t{
    file_descriptor_t fd_array[8];
    uint32_t pid;
    uint32_t parent_pid;

    uint32_t exception_flag;

    uint32_t execute_esp;
    uint32_t execute_ebp;

    uint8_t args[128];
}pcb_t;

/* create the ops tables */
ops_table_t rtc_ops_table;
ops_table_t file_ops_table;
ops_table_t dir_ops_table;
ops_table_t stdin_ops_table;
ops_table_t stdout_ops_table;


/* terminates process, return specific value to parent process */
int32_t syscall_halt(uint8_t status);

/* load/execute program */
int32_t syscall_execute(uint8_t* command);

/* read data from specific location */
int32_t syscall_read(int32_t fd, void* buf, int32_t nbytes);

/* write data to terminal or device */
int32_t syscall_write(int32_t fd, void* buf, int32_t nbytes);

/* allocate file system */
int32_t syscall_open(const uint8_t* filename);

/* free file system and call device driver close */
int32_t syscall_close(int32_t fd);

/* initialize the ops tables */
void ops_table_initialize();

/* bad_call func for stdout */
int32_t bad_read (int32_t fd, void* buf, int32_t nbytes);

/* bad_call func for stdin */
int32_t bad_write (int32_t fd, const void* buf, int32_t nbytes);

/* global variables */
extern uint32_t pid;
extern int globalflag;


#endif


