#include <userspace.h>

void task_system_uart2_send_notifier() {
	int * flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
	int * data = (int *)( UART2_BASE + UART_DATA_OFFSET );
	int recv_tid = -1;
	
	struct short_str_message output_msg;
	output_msg.message_type = PRINT_SERVER_OUTPUT_MESSAGE;
	int str_position = 0;
	output_msg.str[0] = '\0';
	while(1) {
		if(!( *flags & TXFF_MASK )) {
			AwaitEvent(EVENT_UART2_SEND_READY, 0, 0);
		}
		
		if(output_msg.str[str_position] == '\0'){
			//get another message, we have reached the end
			str_position = 0;
			Receive(&recv_tid, (char *) &output_msg, sizeof(output_msg));
			//send the courier back
			Reply(recv_tid, (char *) 0, 0);
		}
		
		//print a character
		*data = (char) output_msg.str[str_position++];
	}
}

void task_system_uart2_recv_notifier() {
	struct short_str_message msg;
	int recv_tid = -1;
	
	msg.message_type = INPUT_SERVER_NOTIFY_MESSAGE;
	
	while(1){
		AwaitEvent(EVENT_UART2_RECEIVE_READY, msg.str, CONFIG_SHORT_STR_MSG_LENGTH);
		
		//get a courier to take the message
		Receive(&recv_tid, (char *) 0, 0);
		
		//send the courier back with the characters
		Reply(recv_tid, (char *) &msg, sizeof(msg));
	}
}

void task_system_uart2_send_server() {
	//my tid
	int my_tid = MyTid();
	
	//messages
	struct extra_long_str_message incoming_msg;
	struct number_message rpl;
	struct short_str_message send_msg;
	
	//create the buffer for output characters
	char cbuff_mem[CONFIG_TERM_SERVER_OUT_BUFFER_SIZE];
	struct char_buffer cbuff;
	cbuffer_init(&cbuff, cbuff_mem, CONFIG_TERM_SERVER_OUT_BUFFER_SIZE );
	
	//create the buffer for couriers
	int courier_buff_mem[CONFIG_TERM_SERVER_OUT_NUM_COURIERS];
	struct int_buffer courier_buff;
	intbuffer_init(&courier_buff, courier_buff_mem, CONFIG_TERM_SERVER_OUT_NUM_COURIERS );
	
	//information about the notifier
	int notifier_tid = 0;
	//int notifier_ready = FALSE;
	
	//register to the name server
	RegisterAs("u2send");
	//create the notifier
	notifier_tid = Create(PRIORITY_TERM_OUT_NOTIFIER,&task_system_uart2_send_notifier);
	
	//used to hijack the output server
	int hijack = -1;
	
	//initialize the couriers
	int i = 0;
	for(i = 0; i < CONFIG_TERM_SERVER_OUT_NUM_COURIERS; i++){
		struct two_number_message courier_init_msg;
		courier_init_msg.message_type = COURIER_INIT_MESSAGE;
		
		//create, initialize, and queue the couriers
		int cour_tid = Create(PRIORTIY_COURIER_HIGH_0, &task_system_courier);
		courier_init_msg.num1 = my_tid;
		courier_init_msg.num2 = notifier_tid;
		
		//send the initialization message
		Send(cour_tid, (char *) &courier_init_msg, sizeof(courier_init_msg), (char *) &rpl, sizeof(rpl));
	}
	
	//receive printing requests / courier ready
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
			case HIJACK_MESSAGE:
				hijack = recv_tid;
				Reply(recv_tid, (char *) 0, 0);
				break;
			case PRINT_SERVER_STRING_MESSAGE:
				//reply immediately, then add the message to the buffer
				rpl.message_type = PRINT_SERVER_STRING_REPLY;
				rpl.num = 0;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				
				//buffer the message
				if(hijack == -1 || hijack == recv_tid){
					cbuffer_push_string( &cbuff, incoming_msg.str, string_size);
				}
				break;
			case COURIER_MESSAGE:
				//buffer the courier
				intbuffer_push( &courier_buff, recv_tid);
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				rpl.num = 0;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
		}
		
		//check if we should be sending a courier
		while(courier_buff.state != INTBUFF_EMPTY && cbuff.state != CBUFF_EMPTY){
			int courier_tid = intbuffer_pop( &courier_buff );
			int max_characters = CONFIG_SHORT_STR_MSG_LENGTH - 1;
			int i = 0;
			
			send_msg.message_type = PRINT_SERVER_OUTPUT_MESSAGE;
			
			//fill the message
			while(max_characters && cbuff.state != CBUFF_EMPTY){
				//copy the character
				send_msg.str[i] = cbuffer_pop( &cbuff );
				//update the indicies
				i++;
				max_characters--;
			}
			//null terminate
			send_msg.str[i] = '\0';
			
			//send the courier
			Reply(courier_tid, (char *) &send_msg, sizeof(send_msg));
		}
	}
}


void task_system_uart2_recv_server() {
	//my tid
	int my_tid = MyTid();
	
	//messages
	struct short_str_message incoming_msg;
	struct char_message rpl;
	
	//create the buffer for input characters
	char cbuff_mem[CONFIG_TERM_SERVER_IN_BUFFER_SIZE];
	struct char_buffer cbuff;
	cbuffer_init(&cbuff, cbuff_mem, CONFIG_TERM_SERVER_IN_BUFFER_SIZE );
	
	//create the buffer for waiting getc'rs
	int getc_tasks_mem[CONFIG_TERM_SERVER_IN_MAX_WAITING_GETC];
	struct int_buffer getc_task_buff;
	intbuffer_init(&getc_task_buff, getc_tasks_mem, CONFIG_TERM_SERVER_IN_MAX_WAITING_GETC );
	
	//information about the notifier
	int notifier_tid = 0;
	//int notifier_ready = FALSE;
	
	//register to the name server
	RegisterAs("u2receive");
	//create the notifier
	notifier_tid = Create(PRIORITY_TERM_IN_NOTIFIER,&task_system_uart2_recv_notifier);
	
	//initialize the couriers
	int i = 0;
	for(i = 0; i < CONFIG_TERM_SERVER_IN_NUM_COURIERS; i++){
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
			case INPUT_SERVER_NOTIFY_MESSAGE:
				//reply immediately, then add the message to the buffer
				Reply(recv_tid, (char *) 0, 0);
				//buffer the message
				cbuffer_push_string( &cbuff, incoming_msg.str, string_size);
				break;
			case INPUT_SERVER_GETC_REQUEST_MESSAGE:
				//buffer the client waiting for a character
				intbuffer_push( &getc_task_buff, recv_tid);
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
		}
		
		//check if we should be replying to a getc request
		while(cbuff.state != CBUFF_EMPTY && getc_task_buff.state != INTBUFF_EMPTY){
			int client_tid = intbuffer_pop( &getc_task_buff );
			
			//build the reply message
			rpl.message_type = INPUT_SERVER_GETC_REPLY_MESSAGE;
			rpl.c = cbuffer_pop( &cbuff );
			
			//reply to the waiting client
			Reply(client_tid, (char *) &rpl, sizeof(rpl));
		}
	}
}


