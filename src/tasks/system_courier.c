#include <userspace.h>

//Generic courier to send messages from one Recieve/Reply task to another
//Only transmits messages in one direction: from the first task to the second
void task_system_courier() {
	int firsttid, secondtid, status, length1, length2, tid;
	char buffer1[MAX_MESSAGE_SIZE];
	struct two_number_message init_msg;
	struct empty_message empty_msg;
	
	//Get message setting up the courier, telling it which tasks it is sending messages between
	status = Receive(&tid, (char *) &init_msg, sizeof(init_msg));
	status = Reply(tid, (char *) 0, 0);
	firsttid = init_msg.num1;
	secondtid = init_msg.num2;
	
	//Set up the first message
	empty_msg.message_type = COURIER_MESSAGE;	
	
	while(1) {
		
		//Get message from first task
		length1 = Send(firsttid, (char *) &empty_msg, sizeof(empty_msg), buffer1, MAX_MESSAGE_SIZE);
		//Send message to second task
		length2 = Send(secondtid, buffer1, length1, (char *) &empty_msg, sizeof(empty_msg));
	}
}

//Generic courier to send messages from one Recieve/Reply task to another
//Includes the ability tp carry the reply of the second task back to the first task
void task_system_2way_courier() {
	int firsttid, secondtid, status, length1, length2, tid;
	char buffer1[MAX_MESSAGE_SIZE];
	char buffer2[MAX_MESSAGE_SIZE];
	struct two_number_message init_msg;
	struct empty_message empty_msg;
	
	//Get message setting up the courier, telling it which tasks it is sending messages between
	status = Receive(&tid, (char *) &init_msg, sizeof(init_msg));
	status = Reply(tid, (char *) 0, 0);
	firsttid = init_msg.num1;
	secondtid = init_msg.num2;
	
	//Set up the first message
	*((int *) buffer2) = COURIER_MESSAGE;
	length2 = sizeof(empty_msg);
	
	while(1) {
		//Send reply from second task to first task, and get message from first task
		length1 = Send(firsttid, buffer2, length2, buffer1, MAX_MESSAGE_SIZE);
		//Send message from first task to second task, get reply from second task
		length2 = Send(secondtid, buffer1, length1, buffer2, MAX_MESSAGE_SIZE);
	}
}

//Generic courier to send a single message to a target
void task_system_single_msg_courier() {
	int sender_tid;
	
	char msg_buffer[MAX_MESSAGE_SIZE];
	
	struct two_number_message init_msg;
	
	//get the initialization message
	Receive(&sender_tid, (char *) &init_msg, sizeof(init_msg));
	Reply(sender_tid, (char *) 0, 0);
	int target_tid = init_msg.num1;
	
	//get the message to transmit
	int length = Receive(&sender_tid, msg_buffer, MAX_MESSAGE_SIZE);
	Reply(sender_tid, (char *) 0, 0);
	
	//send the message to the target
	Send(target_tid, msg_buffer, length, (char *) 0, 0);
	
	//suicide
	Exit();
}

