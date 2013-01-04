#include <userspace.h>

void helper_task_5();

//main task definition
void task_test_timer() {
	//int i = 0;
	int tid = 0, tid3 = 0, tid4 = 0, tid5 = 0, tid6 = 0, status = 0, i = 0;
	struct empty_message msg;
	struct two_number_message rpl;
	
	tid = Create(5,&task_system_nameserver);
	//assert(tid == NAMESERVER_TID,"tid == NAMESERVER_TID");

	RegisterAs("first_task");
	
	tid = Create(5,&task_system_clock_server);
	tid = Create(9,&task_system_clock_notifier);

	tid = Create(0,&task_system_idle);	
		
	bwprintf(COM2, "[FIRST] Creating Prior 1\n");
	tid6 = Create(1,&helper_task_5);
	bwprintf(COM2, "\t[FIRST] Prior 1 has tid [%d]\n",tid6);
	
	bwprintf(COM2, "[FIRST] Creating Prior 2\n");
	tid5 = Create(2,&helper_task_5);
	bwprintf(COM2, "\t[FIRST] Prior 2 has tid [%d]\n",tid5);
	
	bwprintf(COM2, "[FIRST] Creating Prior 3\n");
	tid4 = Create(3,&helper_task_5);
	bwprintf(COM2, "\t[FIRST] Prior 3 has tid [%d]\n",tid4);
	
	bwprintf(COM2, "[FIRST] Creating Prior 4\n");
	tid3 = Create(4,&helper_task_5);
	bwprintf(COM2, "\t[FIRST] Prior 4 has tid [%d]\n",tid3);
	
	for(i = 0; i < 4; i++) {
		//bwprintf(COM2, "[FIRST] Calling Receive\n");
		status = Receive(&tid, (char *) &msg, sizeof(msg));
		assert(status == sizeof(msg),"status == sizeof(msg)")
		//bwprintf(COM2, "\t[FIRST] Received a MSG from [%d]\n",tid);
		
		rpl.message_type = TWO_NUMBER_MESSAGE;
		if(tid == tid6) {
			rpl.num1 = 71;
			rpl.num2 = 3;
		} else if(tid == tid5) {
			rpl.num1 = 33;
			rpl.num2 = 6;
		} else if(tid == tid4) {
			rpl.num1 = 23;
			rpl.num2 = 9;
		} else if(tid == tid3) {
			rpl.num1 = 10;
			rpl.num2 = 20;
		}
		//bwprintf(COM2, "[FIRST] Replying to [%d]\n",tid);
		status = Reply(tid, (char *) &rpl, sizeof(rpl));
		//bwprintf(COM2, "\t[FIRST] Returned From Reply [%d]\n",tid);
		assert(status == 0, "status == 0")
	}
	
	Exit();
}

void helper_task_5() {
	int tid = MyTid(), i = 0, status = 0, f = 0;
	f = WhoIs("first_task");
	struct empty_message msg;
	struct two_number_message rpl;
	msg.message_type = EMPTY_MESSAGE;
	
	//bwprintf(COM2, "[TID %d] GOT FIRST TASKS TID = [%d]\n", tid, f);
	//bwprintf(COM2, "[TID %d] SENDING TO FIRST\n", tid);
	status = Send(f, (char *) &msg, sizeof(msg), (char *) &rpl, sizeof(rpl));
	//assert(status == sizeof(rpl), "status == sizeof(rpl)")
	//bwprintf(COM2, "\t[TID %d] RETURNED FROM SEND WITH STATUS = [%d]\n", tid, status);
	bwprintf(COM2, "\t[TID %d] GOT DELAY INTERVAL = [%d]\n", tid, rpl.num1);

	for(i = 0; i < rpl.num2; i++){
		Delay(rpl.num1);
		bwprintf(COM2, "HelperTask: %d\tDelay Interval:%d\tDelay Number\t%d\r\n", tid, rpl.num1, i);	
	}
	
	Exit();
}

