#include <userspace.h>


void task_system_clock_notifier(){
	struct short_str_message msg;
	int recv_tid = -1;
	
	msg.message_type = CLOCK_TICK_MESSAGE;
	
	while(1){
		AwaitEvent(EVENT_CLOCK_TICK, msg.str, CONFIG_SHORT_STR_MSG_LENGTH);
		
		//get a courier to take the message
		Receive(&recv_tid, (char *) 0, 0);
		
		//send the courier back with the characters
		Reply(recv_tid, (char *) &msg, sizeof(msg));
	}
}

void task_system_clock_server() {
	int time = 0; //In 10ms clock ticks
	
	struct heap task_heap;

	int elements[CONFIG_CLK_MAX_DELAYED_TASKS], priorities[CONFIG_CLK_MAX_DELAYED_TASKS];
	task_heap.size = 0;
	task_heap.max_size = CONFIG_CLK_MAX_DELAYED_TASKS;
	task_heap.elements = elements;
	task_heap.priorities = priorities;
	
	//my tid
	int my_tid = MyTid();
	
	//messages
	struct short_str_message incoming_msg;
	struct number_message* num_msg;
	struct number_message rpl;
	
	//information about the notifier
	int notifier_tid = 0;
	//int notifier_ready = FALSE;
	
	//register to the name server
	RegisterAs("clock_serv");
	//create the notifier
	notifier_tid = Create(PRIORITY_CLOCK_NOTIFIER,&task_system_clock_notifier);
	
	//initialize the couriers
	int i = 0;
	for(i = 0; i < CONFIG_CLK_NUM_COURIERS; i++){
		struct two_number_message courier_init_msg;
		courier_init_msg.message_type = COURIER_INIT_MESSAGE;
		
		//create, initialize, and queue the couriers
		int cour_tid = Create(PRIORTIY_COURIER_HIGH_0, &task_system_courier);
		courier_init_msg.num1 = notifier_tid;
		courier_init_msg.num2 = my_tid;
		
		//send the initialization message
		Send(cour_tid, (char *) &courier_init_msg, sizeof(courier_init_msg), (char *) &rpl, sizeof(rpl));
	}
	
	//receive notifier messages / getc requests
	while(1) {
		int recv_size = 0;
		int recv_tid = -1;
		//get the next request
		recv_size = Receive(&recv_tid, (char *) &incoming_msg, sizeof(incoming_msg));
		
		//calculate the length of the string we received
		int string_size = recv_size - sizeof(int);
		if(string_size < 0){ string_size = 0; }
		
		//act on the message type
		switch(incoming_msg.message_type){
			case CLOCK_TICK_MESSAGE:
				//reply immediately, then add the message to the buffer
				Reply(recv_tid, (char *) 0, 0);
				int j = 0;
				while(incoming_msg.str[j] != '\0'){
					j++;
				}
				time = time + j;
				break;
			case CLOCK_TIME_MESSAGE:
				rpl.message_type = CLOCK_TIME_REPLY;
				rpl.num = time;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				break;
			case CLOCK_DELAY_MESSAGE:
				//bwprintf(COM2, "%d %d\r\n", tid, msg.num);
				num_msg = (struct number_message*) &incoming_msg;
				heap_add(&task_heap, recv_tid, num_msg->num + time);
				break;
			case CLOCK_DELAY_UNTIL_MESSAGE:
				//bwprintf(COM2, "%d %d\r\n", tid, msg.num);
				num_msg = (struct number_message*) &incoming_msg;
				heap_add(&task_heap, recv_tid, num_msg->num);
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
		}
		while(task_heap.size > 0 && task_heap.priorities[0] <= time) {
			rpl.message_type = CLOCK_WAKEUP;
			Reply(heap_remove_min(&task_heap),(char *) &rpl, sizeof(rpl));
		}
	}
}
