#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "i8259.h"

/* Port Values */
#define keyboard_irq 0x1
#define KEYBOARD_PORT 0x60 
#define KEYBOARD_STATUS 0x64

/* Variables of Sizes */
#define BUFFER_SIZE 128
#define ARRAY_SIZE 48
#define EXTENDED_BUFFER 256

/* Useful Scancodes */
#define RIGHT_SHIFT 0x36
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT_RELEASE 0xB6
#define LEFT_SHIFT_RELEASE 0xAA
#define L_SCANCODE 0x26
#define CTRL_PRESS 0x1D
#define CTRL_RELEASE 0x9D
#define BACKSPACE_SCANCODE 0x0E
#define CAPS_PRESS 0x3A
#define TAB_SCANCODE 0x0F
#define ENTER_SCANCODE  0x1C
#define ENTER_RELEASE 0x9C
#define ALT_PRESS 0x38
#define ALT_RELEASE 0xB8
#define F1_SCANCODE 0x3B
#define F2_SCANCODE 0x3C
#define F3_SCANCODE 0x3D



/* Keyboard Driver Functions */
extern void init_keyboard();
extern void keyboard_interrupt();
extern void clear_keyboard_buffer();
extern void printToScreen(char key_in);

/* Keybuffer that holds our data typed */
volatile extern char key_buffer[EXTENDED_BUFFER];

/* enter flag for when user presses enter */
volatile extern int32_t enter_flag;

/* current index of buffer */
extern int buffer_index;


#endif
