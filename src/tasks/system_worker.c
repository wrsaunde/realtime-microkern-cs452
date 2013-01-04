#include <userspace.h>

//task to delay, and then send to the calling task
void task_system_worker_delay() {
	int sender_tid;
	
	struct two_number_message init_msg;
	
	//get the initialization message
	Receive(&sender_tid, (char *) &init_msg, sizeof(init_msg));
	Reply(sender_tid, (char *) 0, 0);
	int type = init_msg.num1;
	int length = init_msg.num2;
	
	if(type == 1){
		DelayUntil(length);
	}else{
		Delay(length);
	}
	
	init_msg.message_type = DELAY_WORKER_MESSAGE;
	
	//wake the calling task up
	Send(sender_tid, (char*) &init_msg, sizeof(init_msg), (char *) 0, 0);
	
	//suicide
	Exit();
}

//task to delay, and then send to the calling task
void task_system_worker_delay_with_id() {
	int sender_tid;
	
	struct two_number_message init_msg;
	
	//get the initialization message
	Receive(&sender_tid, (char *) &init_msg, sizeof(init_msg));
	Reply(sender_tid, (char *) 0, 0);
	int id = init_msg.num1;
	int length = init_msg.num2;
	
	Delay(length);
	
	init_msg.message_type = DELAY_WORKER_MESSAGE;
	init_msg.num1 = id;
	init_msg.num2 = length;
	
	//wake the calling task up
	Send(sender_tid, (char*) &init_msg, sizeof(init_msg), (char *) 0, 0);
	
	//suicide
	Exit();
}

//task to delay, and then send to the calling task
void task_system_worker_periodic_delay() {
	int sender_tid;
	
	struct two_number_message init_msg;
	
	//get the initialization message
	Receive(&sender_tid, (char *) &init_msg, sizeof(init_msg));
	Reply(sender_tid, (char *) 0, 0);
	int type = init_msg.num1;
	int length = init_msg.num2;
	
	while(1){
		Delay(length);
	
		init_msg.message_type = DELAY_WORKER_MESSAGE;
	
		//wake the calling task up
		Send(sender_tid, (char*) &init_msg, sizeof(init_msg), (char *) 0, 0);
	}
	
	//suicide
	Exit();
}

//task to delay, and then send to the calling task
void task_system_worker_delay_until_with_id() {
	int sender_tid;
	
	struct two_number_message init_msg;
	
	//get the initialization message
	Receive(&sender_tid, (char *) &init_msg, sizeof(init_msg));
	Reply(sender_tid, (char *) 0, 0);
	int id = init_msg.num1;
	int length = init_msg.num2;
	
	DelayUntil(length);
	
	init_msg.message_type = DELAY_WORKER_MESSAGE;
	init_msg.num1 = id;
	init_msg.num2 = length;
	
	//wake the calling task up
	Send(sender_tid, (char*) &init_msg, sizeof(init_msg), (char *) 0, 0);
	
	//suicide
	Exit();
}
