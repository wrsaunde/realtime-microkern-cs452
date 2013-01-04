/* task_timing_test.c - code to test send recieve reply timings */
#include <userspace.h>

#define NAME_NOT_FOUND -3
#define NAME_DUPLICATE -4

#define PRIORITY_USED 4

void time_test_receiver_4();
void time_test_receiver_64();

void reset_timer(unsigned int* t3){	
	//disable the timer
	t3[2] = 0;
	//load the new value
	t3[0] = 0xFFFFFF;
	//enable the timer in periodic mode at 508kHz ticks
	t3[2] = ENABLE_MASK | MODE_MASK | CLKSEL_MASK;
}

void task_test_srr_timing(){
	//should be executed at priority PRIORITY_USED in order to function correctly
	unsigned int time1 = 0;
	unsigned int time2 = 0;
	unsigned int timediff = 0;

	
	//set up the timer
	unsigned int* t3 = (unsigned int*) TIMER3_BASE;	//get the timer address
	reset_timer(t3);
	
	//first check 4 bytes
	bwprintf(COM2, "\n[TIMING_TEST] START 4 BYTE TEST\n");
	char msg1[4];
	int tid = 0;
	int msglen = sizeof(msg1);
	
	bwprintf(COM2, "\t[TIMING_TEST] CREATING RECEIVER\n");
	int recv_tid =  Create(PRIORITY_USED, &time_test_receiver_4);
	Pass();
	bwprintf(COM2, "\t[TIMING_TEST] RECVR SHOULD BE READY, START TEST\n");
	
	reset_timer(t3);
	time1 = t3[1];
	Send( recv_tid, msg1, msglen, msg1, msglen );
	time2 = t3[1];
	timediff = time1 - time2;
	
	bwprintf(COM2, "\t[TIMING_TEST] TIMING TAKEN, PASSING TO LET RECV EXIT\n");
	Pass();
	
	bwprintf(COM2, "\t[TIMING_TEST] TIME1 IS [%u]\n", time1);
	bwprintf(COM2, "\t[TIMING_TEST] TIME2 IS [%u]\n", time2);
	bwprintf(COM2, "\t[TIMING_TEST] COMPLETED ROUND TRIP IN [%u] TICKS (508kHz)\n", timediff);
	bwprintf(COM2, "[TIMING_TEST] 4 BYTE TEST COMPLETE\n");
	
	//first check 64 bytes
	bwprintf(COM2, "\n[TIMING_TEST] START 64 BYTE TEST\n");
	char msg2[64];
	tid = 0;
	msglen = sizeof(msg2);
	
	bwprintf(COM2, "\t[TIMING_TEST] CREATING RECEIVER\n");
	recv_tid =  Create(PRIORITY_USED, &time_test_receiver_64);
	Pass();
	bwprintf(COM2, "\t[TIMING_TEST] RECVR SHOULD BE READY, START TEST\n");
	
	//reset_timer(t3);
	time1 = t3[1];
	Send( recv_tid, msg2, msglen, msg2, msglen );
	time2 = t3[1];
	timediff = time1 - time2;
	
	bwprintf(COM2, "\t[TIMING_TEST] TIMING TAKEN, PASSING TO LET RECV EXIT\n");
	Pass();
	
	bwprintf(COM2, "\t[TIMING_TEST] TIME1 IS [%u]\n", time1);
	bwprintf(COM2, "\t[TIMING_TEST] TIME2 IS [%u]\n", time2);
	bwprintf(COM2, "\t[TIMING_TEST] COMPLETED ROUND TRIP IN [%u] TICKS (508kHz)\n", timediff);
	bwprintf(COM2, "[TIMING_TEST] 64 BYTE TEST COMPLETE\n");
	
	Exit();
}

void time_test_receiver_4(){
	char msg[4];
	int tid = 0;
	int msglen = sizeof(msg);
	
	bwprintf(COM2, "\t[TIMING_TEST_RECVR] CALLING RECEIVE\n");
	Receive( &tid, msg, msglen);
	Reply( tid, msg, msglen );
	
	bwprintf(COM2, "\t[TIMING_TEST_RECVR] EXITING\n");
	Exit();
}

void time_test_receiver_64(){
	char msg[64];
	int tid = 0;
	int msglen = sizeof(msg);
	
	bwprintf(COM2, "\t[TIMING_TEST_RECVR] CALLING RECEIVE\n");
	Receive( &tid, msg, msglen);
	Reply( tid, msg, msglen );
	
	bwprintf(COM2, "\t[TIMING_TEST_RECVR] EXITING\n");
	Exit();
}
