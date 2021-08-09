#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "rtc.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
*/
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}


/* exception_test()
 *
 * Asserts exception handling
 * Inputs: None
 * Outputs: FAIL and prints excception error
 * Side Effects: exception handler gets raised
 * Coverage: initialize idt and the exception handler
 * Files: idt.c/h
*/
int exception_test() {
	TEST_HEADER;
	asm volatile("int $0x80");
	return FAIL;
}

/* exception_test2()
 *
 * Asserts exceptions
 * Inputs: None
 * Outputs: FAIL and prints excception error
 * Side Effects: exception handler gets raised
 * Coverage: initialize idt and the exception handler
 * Files: idt.c/h
*/
int exception_test2() {
	TEST_HEADER;
	asm volatile("int $15"); //exceptions
	return FAIL;
}

/* exception_division()
 *
 * Asserts division vy zero handler
 * Inputs: None
 * Outputs: PASS and prints excception error
 * Side Effects: div by 0 handler gets raised
 * Coverage: initialize idt and the interrupt handler
 * Files: idt.c/h
*/
int exception_division() {
	TEST_HEADER;
	int a, b;
	a = 200;
	b = 0;
	a = a/b;
	return FAIL;
}

/* accessible_paging_test1
 *
 * Asserts that vid memory lower bound can be dereferenced
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (shouldn't happen)
 * Coverage: vid mem in physical mem from paging
 * Files: paging.c/h
*/
int accessible_paging_test1() {
	TEST_HEADER;
	int *location;
	int retval;

	location = (int*)0xB8000;
	retval = *location;
	printf("%d\n", *location);
	return PASS;
}

/* accessible_paging_test2
 *
 * Asserts that vid memory upper bound can be dereferenced
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (shouldn't happen)
 * Coverage: vid mem in physical mem from paging
 * Files: paging.c/h
*/
int accessible_paging_test2() {
	TEST_HEADER;
	char *location;
	char retval;

	location = (char*)0xB8FFF;
	retval = *location;
	printf("%c\n", *location);
	return PASS;
}

/* accessible_paging_test3
 *
 * Asserts that kernel memory lower bound can be dereferenced
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (shouldn't happen)
 * Coverage: kernel mem in physical mem from paging
 * Files: paging.c/h
*/
int accessible_paging_test3() {
	TEST_HEADER;
	int *location;
	int retval;

	location = (int*)0x400000;
	retval = *location;
	printf("%d\n", *location);
	return PASS;
}

/* accessible_paging_test4
 *
 * Asserts that kernel memory upper bound can be referenced
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (shouldn't happen)
 * Coverage: kernel mem in physical mem from paging
 * Files: paging.c/h
*/
int accessible_paging_test4() {
	TEST_HEADER;
	char *location;
	char retval;

	location = (char*)0x7FFFFF;
	retval = *location;
	printf("%c\n", *location);
	return PASS;
}

/* inaccessible_paging_test1
 *
 * Asserts that vid memory lower bound causes fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (should happen)
 * Coverage: vid mem in physical mem from paging
 * Files: paging.c/h
*/
int inaccessible_paging_test1() {
	TEST_HEADER;
	int *location;
	int retval;

	location = (int*)0xB6000;
	retval = *location;
	printf("%d\n", *location);
	return FAIL;
}

/* inaccessible_paging_test2
 *
 * Asserts that vid memory upper bound causes fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (should happen)
 * Coverage: vid mem in physical mem from paging
 * Files: paging.c/h
*/
int inaccessible_paging_test2() {
	TEST_HEADER;
	int *location;
	int retval;

	location = (int*)0xC8000;
	retval = *location;
	printf("%d\n", *location);
	return FAIL;
}

/* inaccessible_paging_test3
 *
 * Asserts that kernel memory lower bound causes fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (should happen)
 * Coverage: kernel mem in physical mem from paging
 * Files: paging.c/h
*/
int inaccessible_paging_test3() {
	TEST_HEADER;
	int *location;
	int retval;

	location = (int*)0x200000;
	retval = *location;
	printf("%d\n", *location);
	return FAIL;
}

/* inaccessible_paging_test4
 *
 * Asserts that kernel memory upper bound causes fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (should happen)
 * Coverage: kernel mem in physical mem from paging
 * Files: paging.c/h
*/
int inaccessible_paging_test4() {
	TEST_HEADER;
	int *location;
	int retval;

	location = (int*)0x8FFFFF;
	retval = *location;
	printf("%d\n", *location);
	return FAIL;
}

/* inaccessible_paging_test5
 *
 * Asserts that deref 0 causes fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays fault message if any (should happen)
 * Coverage: page fault handlin for null location
 * Files: paging.c/h
*/
int inaccessible_paging_test5() {
	TEST_HEADER;
	int *location;
	int retval;

	location = NULL;
	retval = *location;
	printf("%d\n", *location);
	return FAIL;
}

/* Checkpoint 2 tests */

/* rtc_functions
 *
 * Asserts that a non power of 2 frequency fails
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: displays error message for non power of 2 frequency
 * Coverage: Setting RTC frequency
 * Files: rtc.c/h
*/
int rtc_functions(){

	TEST_HEADER;
	char* rtc ="rtc";
	int32_t fd = open_rtc((const uint8_t*)rtc); //set frequency to 2
	
	uint32_t buf[4];
	uint32_t buf_temp;
	*buf = 400;
	buf_temp= write_rtc(fd, buf, 4);
	if(buf_temp == -1){ 

		printf("NOT A POWER OF TWO OR OUTSIDE BOUNDS \n");
		return FAIL;
	}
	else {
		
		printf("SHOULDN'T REACH HERE \n");
		return PASS;
	}
	
}

/* rtc_inbounds
 *
 * Asserts that RTC frequency is getting written to, and interrupts are generated
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Visualizes the frequency rate thru printing i's 
 * Coverage: RTC frequency updates, and interrupts
 * Files: rtc.c/.h
*/
int rtc_inbounds(){
	TEST_HEADER;



	char* rtc ="rtc";
	int32_t fd = open_rtc((const uint8_t*)rtc); //set frequency to 2
	
	uint32_t buf[4];
	uint32_t buf_temp;
	for(*buf= 2; *buf <= 1024; *buf *= 2){
		buf_temp= write_rtc(fd, buf, 4);

		int i = *buf;
	
		int j;

		if(buf_temp != 0){ //test if can write freq thats out of bounds
			printf("test failed \n");
			return FAIL;
		}
		else {
			printf("RTC Frequency %d \n", i);
			
		}

		for(j = 0; j < 25; j++){

			read_rtc(fd, buf, 4);

			printf("i");
		}

		printf("\n");


	}

	return PASS;
}




/* terminal_test
 *
 * checks the bounds of terminal buffer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: writes to terminal based on buffer size
 * Coverage: terminal read/write issues
 * Files: terminal.c/.h, keyboard.c/.h
*/
int terminal_test() {
	TEST_HEADER;
	char buffer[256]; //account for upper bound test case as well
	int nbytes;

	int done = 0;

	while(!done) {
		terminal_write(0,"Lower bound check\n", 18);
		nbytes = terminal_read(0, buffer, 18);
		terminal_write(0, buffer, nbytes);
		terminal_write(0,"Upper bound check\n", 18);
		nbytes = terminal_read(0, buffer, 128);
		terminal_write(0, buffer, nbytes);
		terminal_write(0,"Out of bound check\n", 19);
		nbytes = terminal_read(0, buffer, 228);
		terminal_write(0, buffer, nbytes);

		done++;
	}
	return PASS;
}


void boot_image(int32_t filename) {
	char* rtc ="rtc";
	int32_t fd = open_rtc((const uint8_t*)rtc); //set frequency to 2
	
	uint32_t buf[4];
	uint32_t buf_temp;
	*buf = 2;
	buf_temp= write_rtc(fd, buf, 4);

	// int i = *buf;

	int j;
	for(j = 0; j < 5; j++){

		read_rtc(fd, buf, 4);
		clear();
		file_test(filename);
	}

}





/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */




/* Test suite entry point */
void launch_tests(){

// uint8_t filepretty[] = "bootimage.txt";
// boot_image(filepretty);

/* CHECKPOINT 2 TESTS */
// uint8_t test_fname[] = "frame0.txt";
//	TEST_OUTPUT("rtc_freqcycle", rtc_inbounds());
//	TEST_OUTPUT("terminal_test", terminal_test());
//	TEST_OUTPUT("rtc_nonpower2", rtc_functions());
//	TEST_OUTPUT("directory_test", directory_test());
//	TEST_OUTPUT("file_name_test", file_test(test_fname));


/* CHECKPOINT 1 TESTS */

//	TEST_OUTPUT("idt_test", idt_test());
//	TEST_OUTPUT("exception", exception_test());
//	TEST_OUTPUT("exception2", exception_test());
//	TEST_OUTPUT("division", exception_division());
//	TEST_OUTPUT("acc_p1", accessible_paging_test1());
//	TEST_OUTPUT("acc_p2", accessible_paging_test2());
//	TEST_OUTPUT("acc_p3", accessible_paging_test3());
//	TEST_OUTPUT("acc_p4", accessible_paging_test4());
//	TEST_OUTPUT("inacc_p1", inaccessible_paging_test1());
//	TEST_OUTPUT("inacc_p2", inaccessible_paging_test2());
//	TEST_OUTPUT("inacc_p3", inaccessible_paging_test3());
//	TEST_OUTPUT("inacc_p4", inaccessible_paging_test4());
//	TEST_OUTPUT("inacc_p5", inaccessible_paging_test5());
	// launch your tests here

}
