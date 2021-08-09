#include "lib.h"
#include "keyboard.h"


#define BUFFER_SIZE 128         // Max buffer size for terminals
#define MAX_TERMINALS 3         // Max number of terminals



/* Terminal Structure */
typedef struct term{
    int process_active;     // tells the terminal which process ID is active
    int cursor_x;           // used to reset cursor location
    int cursor_y;
    uint8_t key_buffer[BUFFER_SIZE];      // each terminal needs its own keyboard buffer
    uint8_t enter_flag;     // enter flag lets us know to stop taking input
    uint8_t* video_mem;     // video mem ptr - changes as we set it to active term/backup

    uint32_t parent_pid;    // parent process PID
    uint32_t visible_pid;   // visible process PID

} terminal_t;


terminal_t term_blk[3];     // create three terminals

extern int curr_active_terminal;      // global variable to determine which terminal is currently active


/* Terminal Interface Functions */
int32_t terminal_open();
int32_t terminal_close();
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* Terminal Switching Functions */
extern void init_terminal();
extern int32_t terminal_swap(int new_term_val);
int32_t save_visible_terminal(int terminal_old);
int32_t restore_new_terminal(int terminal_new);

