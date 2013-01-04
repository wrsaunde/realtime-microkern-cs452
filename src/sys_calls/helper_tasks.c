#include <userspace.h>

//Create a courier to go from tid1 to tid2, returning the new courier's TID
int Courier(int tid1, int tid2) {
	struct two_number_message courier_init_msg;
	struct empty_message rpl;
	int status, cour_tid;
	
	courier_init_msg.message_type = COURIER_INIT_MESSAGE;
	
	//create, initialize, and queue the courier
	cour_tid = Create(PRIORTIY_COURIER_HIGH_0, &task_system_courier);
	
	if(cour_tid < 0) {
		return -1;
	}
	
	courier_init_msg.num1 = tid1;
	courier_init_msg.num2 = tid2;
	
	//send the initialization message
	status = Send(cour_tid, (char *) &courier_init_msg, sizeof(courier_init_msg), (char *) &rpl, sizeof(rpl));
	
	if(status < 0) {
		return -2;
	}
	
	return cour_tid;
}

//Create a train task and return the trains TID
int Train(int train_number, int track){
	struct two_number_message train_init_msg;
	struct empty_message rpl;
	int status, train_tid;
	
	train_init_msg.message_type = TRAIN_INIT_MESSAGE;
	
	//create, initialize, and queue the courier
	train_tid = Create(PRIORITY_USER_TRAIN, &task_user_train);
	if(train_tid < 0) {
		return -1;
	}
	
	train_init_msg.num1 = train_number;
	train_init_msg.num2 = track;
	
	//send the initialization message
	status = Send(train_tid, (char *) &train_init_msg, sizeof(train_init_msg), (char *) &rpl, sizeof(rpl));
	
	if(status < 0) {
		return -2;
	}
	
	return train_tid;
}
