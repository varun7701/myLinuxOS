#include "lib.h"
#include "exceptions.h"
#include "syscall.h"

/* maps exception to its correct function */
void exception0() { exceptions(0); }
void exception1() { exceptions(1); }
void exception2() { exceptions(2); }
void exception3() { exceptions(3); }
void exception4() { exceptions(4); }
void exception5() { exceptions(5); }
void exception6() { exceptions(6); }
void exception7() { exceptions(7); }
void exception8() { exceptions(8); }
void exception9() { exceptions(9); }
void exception10() { exceptions(10); }
void exception11() { exceptions(11); }
void exception12() { exceptions(12); }
void exception13() { exceptions(13); }
void exception14() { exceptions(14); }
void exception15() { exceptions(15); }
void exception16() { exceptions(16); }
void exception17() { exceptions(17); }
void exception18() { exceptions(18); }
void exception19() { exceptions(19); }

/* void exceptions(int entry)
 * Inputs: entry - which exception to handle
 * Outputs: none
 * Side Effects: prints exception to console, squashes user-program
*/
void exceptions(int entry)
{
  switch(entry)
  {
    case 0:
      printf("EXCEPTION: Divide by Zero");
      break;
    case 1:
      printf("EXCEPTION: Debug");
      break;
    case 2:
      printf("EXCEPTION: NMI Interrupt");
      break;
    case 3:
      printf("EXCEPTION: Breakpoint");
      break;
    case 4:
      printf("EXCEPTION: OverFlow");
      break;
    case 5:
      printf("EXCEPTION: BOUND Range Exceed");
      break;
    case 6:
      printf("EXCEPTION: Invalid Opcode");
      break;
    case 7:
      printf("EXCEPTION: Device Not Available");
      break;
    case 8:
      printf("EXCEPTION: Double Fault");
      break;
    case 9:
      printf("EXCEPTION: Coprocessor Segment Overrun");
      break;
    case 10:
      printf("EXCEPTION: Invalid TSS");
      break;
    case 11:
      printf("EXCEPTION: Segment Not Present");
      break;
    case 12:
      printf("EXCEPTION: Stack Fault");
      break;
    case 13:
      printf("EXCEPTION: General Protection");
      break;
    case 14:
      printf("EXCEPTION: Page-Fault Exception");
      break;
    case 15:
      printf("EXCEPTION: Intel Reserved");
      break;
    case 16:
      printf("EXCEPTION: FPU Floating Point");
      break;
    case 17:
      printf("EXCEPTION: Alignment Check");
      break;
    case 18:
      printf("EXCEPTION: Machine-Check");
      break;
    case 19:
      printf("EXCEPTION: SIMD Floatin Point");
      break;

  }
    printf("\n");

    // set our global flag so halt knows an exception was raised
    globalflag = ONE;

    syscall_halt(EXCEPTION_STATUS);      //Squashing user program

}
