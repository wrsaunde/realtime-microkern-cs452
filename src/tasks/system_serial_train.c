#include <userspace.h>
//7960A240

void task_system_uart1_send_notifier() {
	int * flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
	int * data = (int *)( UART1_BASE + UART_DATA_OFFSET );
	int * mdmctl = (int *)( UART1_BASE + UART_MDMCTL_OFFSET);
	int * mdmsts = (int *) (UART1_BASE + UART_MDMSTS_OFFSET);
	
	int recv_tid;
	
	struct char_message msg;
	
	//-1: 	CTS is 0, can't send, 
	//0: 	initial state
	//1 	CTS is on after having been de-asserted, we can send now
	int cts_state = 0;
	int transmit_state = 0;	
	int count = 0;
	int first = 0;
	
	int flagdata = 0, mdmstsdata = 0;
	
	while(1) {
		//get another message, we have reached the end
		Receive(&recv_tid, (char *) &msg, sizeof(msg));
		//send the courier back
		Reply(recv_tid, (char *) 0, 0);		
		
		//Set proper modem control bits for transmission
		*mdmctl = *mdmctl | 3;
		
		cts_state = 0;
		transmit_state = 0;		
		count = 0;

		//Putc(COM2,'9');
		
		while(1) {
			//Putc(COM2,'0');
			
			//Read volatile data
			
			if(transmit_state == 0 || count == 0) {
				AwaitEvent(EVENT_UART1_SEND_READY_INITIAL, 0, 0);
			} else {
				AwaitEvent(EVENT_UART1_SEND_READY, 0, 0);				
			}

			mdmstsdata = *mdmsts;			
			flagdata = *flags;

			count++;
			//Putc(COM2, 'A');
			
			if( ((mdmstsdata & DCTS_MASK) || first == 0) && (flagdata & CTS_MASK)) {
				//Putc(COM2,'2');				
				cts_state = 1;
			}
			
			if(cts_state == 0 && !(flagdata & CTS_MASK)) {
				//Putc(COM2,'1');
				//See CTS de-asserted: wait for it to be reasserted
				cts_state = -1;
			}			
			
			if(cts_state == -1) {
				if((flagdata & CTS_MASK) ) {
					//Putc(COM2,'3');				
					//CTS has been re-asserted, let's send
					cts_state = 1;
				}
			}
			
			if((flagdata & TXFE_MASK)) {
				//Putc(COM2, '4');				
				transmit_state = 1;			
			}
			
			
			if(cts_state == 1 && transmit_state == 1) {
				//CTS and transmit flags are in the right state
				//Write the data and reset
				*data = (char) msg.c;				
				//Putc(COM2, '5');
				first = 1;				
				break;
			}
			
		}
	}
}

void task_system_uart1_recv_notifier() {
	int recv_tid;
	
	struct empty_message msg;
	struct char_message rpl;
	
	char buffer[CONFIG_SHORT_STR_MSG_LENGTH];
	
	while(1) {		
		AwaitEvent(EVENT_UART1_RECEIVE_READY, buffer, CONFIG_SHORT_STR_MSG_LENGTH);
		
		rpl.message_type = UART1_INPUT_SERVER_NOTIFY_MESSAGE;		
		rpl.c = buffer[0];
		
		//grab a courier
		Receive(&recv_tid, (char *) &msg, sizeof(msg));		
		//send the courier back to the server with the new character
		Reply(recv_tid, (char *) &rpl, sizeof(rpl));
	}
}



void task_system_uart1_send_server() {
	//my tid
	int my_tid = MyTid();
	
	//messages
	struct char_message incoming_msg;
	struct number_message rpl;
	struct char_message send_msg;
	
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
	RegisterAs("u1send");
	
	//create the notifier
	notifier_tid = Create(PRIORITY_TRAIN_OUT_NOTIFIER,&task_system_uart1_send_notifier);

	//initialize the couriers
	int i = 0;
	for(i = 0; i < CONFIG_TERM_SERVER_OUT_NUM_COURIERS; i++){
		Courier(my_tid, notifier_tid);		
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
		switch(incoming_msg.message_type) {
			case UART1_PRINT_SERVER_INPUT_MESSAGE:
				//reply immediately, then add the message to the buffer
				rpl.message_type = UART1_PRINT_SERVER_INPUT_REPLY;
				rpl.num = 0;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				
				//buffer the message
				cbuffer_push_char( &cbuff, incoming_msg.c);
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
			
			send_msg.message_type = UART1_PRINT_SERVER_OUTPUT_MESSAGE;
			
			send_msg.c = cbuffer_pop( &cbuff );
			
			//send the courier
			Reply(courier_tid, (char *) &send_msg, sizeof(send_msg));
		}
	}
}





void task_system_uart1_recv_server() {
	//my tid
	int my_tid = MyTid();
	
	//messages
	struct char_message incoming_msg;
	struct char_message char_rpl;
	struct number_message rpl;
	
	//create the buffer for input characters
	char cbuff_mem[CONFIG_UART1_SERVER_IN_BUFFER_SIZE];
	struct char_buffer cbuff;
	cbuffer_init(&cbuff, cbuff_mem, CONFIG_UART1_SERVER_IN_BUFFER_SIZE );
	
	//create the buffer for waiting getc'rs
	int getc_tasks_mem[CONFIG_UART1_SERVER_IN_MAX_WAITING_GETC];
	struct int_buffer getc_task_buff;
	intbuffer_init(&getc_task_buff, getc_tasks_mem, CONFIG_UART1_SERVER_IN_MAX_WAITING_GETC );
	
	//information about the notifier
	int notifier_tid = 0;
	//int notifier_ready = FALSE;
	
	//register to the name server
	RegisterAs("u1receive");
	//create the notifier
	notifier_tid = Create(PRIORITY_TRAIN_IN_NOTIFIER,&task_system_uart1_recv_notifier);
	
	//initialize the couriers
	int i = 0;
	for(i = 0; i < CONFIG_UART1_SERVER_IN_NUM_COURIERS; i++){
		Courier(notifier_tid, my_tid);		
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
			case UART1_INPUT_SERVER_NOTIFY_MESSAGE:
				//reply immediately, then add the message to the buffer
				Reply(recv_tid, (char *) 0, 0);
				//buffer the message
				cbuffer_push_char( &cbuff, incoming_msg.c);
				break;
			case UART1_INPUT_SERVER_GETC_REQUEST_MESSAGE:
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
			char_rpl.message_type = UART1_INPUT_SERVER_GETC_REPLY_MESSAGE;
			char_rpl.c = cbuffer_pop( &cbuff );
			
			//reply to the waiting client
			Reply(client_tid, (char *) &char_rpl, sizeof(char_rpl));
		}
	}
}




