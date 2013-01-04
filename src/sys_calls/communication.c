#include <userspace.h>

//Create a courier to go from tid1 to tid2, returning the new courier's TID
int CourierSend(int Tid, char *msg, int msglen){
	struct two_number_message courier_init_msg;
	int status, cour_tid;
	
	courier_init_msg.message_type = COURIER_INIT_MESSAGE;
	
	//create, initialize, and queue the courier
	cour_tid = Create(PRIORTIY_COURIER_HIGH_0, &task_system_single_msg_courier);
	
	if(cour_tid < 0) {
		return -1;
	}
	
	courier_init_msg.num1 = Tid;
	courier_init_msg.num2 = 0;
	
	//send the initialization message
	status = Send(cour_tid, (char *) &courier_init_msg, sizeof(courier_init_msg), (char *) 0, 0);
	
	if(status < 0) {
		return -2;
	}
	
	//send the message to transmit
	status = Send(cour_tid, msg, msglen, (char *) 0, 0);
	
	if(status < 0) {
		return -3;
	}
	
	return 0;
}

