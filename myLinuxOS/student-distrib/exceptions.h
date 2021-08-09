#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#define EXCEPTION_STATUS 0                      // value passed into syscall_halt
#define ONE 1                                   // used to set global flag

//functions that call exceptions(int entry) with switch value to print exceptions
void exception0();
void exception1();
void exception2();
void exception3();
void exception4();
void exception5();
void exception6();
void exception7();
void exception8();
void exception9();
void exception10();
void exception11();
void exception12();
void exception13();
void exception14();
void exception15();
void exception16();
void exception17();
void exception18();
void exception19();


//switch statement function to print out exceptions
void exceptions(int entry);



#endif
