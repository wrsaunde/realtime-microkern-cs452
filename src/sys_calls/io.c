/**
 *
 * io.c
 *
 **/

#include <userspace.h>

int Getc( int channel ){	
	int tid = -1;
	struct empty_message req_msg;
	
	struct char_message rpl;
	
	if(channel == COM1){
		//we are sending to train, get the tid
		tid = WhoIs("u1receive");
		req_msg.message_type = UART1_INPUT_SERVER_GETC_REQUEST_MESSAGE;
	}else if(channel == COM2){
		//we are sending to terminal, get the tid
		tid = WhoIs("u2receive");
		req_msg.message_type = INPUT_SERVER_GETC_REQUEST_MESSAGE;		
	}else{
		return -3; //NOT A PROPER COM CHANNEL
	}
	
	//check the return value of WhoIs
	if(tid < 0){ return -1; }
	
	//send the message to the server
	//int status = Send(tid, (char *) &req_msg, sizeof(req_msg), (char *) &rpl, sizeof(rpl));
	Send(tid, (char *) &req_msg, sizeof(req_msg), (char *) &rpl, sizeof(rpl));
	
	//TODO: add in checking for the return of the send and the reply
	//success
	return (int) rpl.c;
}

int Putc( int channel, char ch ){
	int tid = -1;
	int copy_length = 0;
	//int status = 0;
	
	struct number_message rpl;
	
	if(channel == COM1) {
		//we are sending to train, get the tid
		tid = WhoIs("u1send");

		//check the return value of WhoIs
		if(tid < 0){ return -1; }

		struct char_message msg;

		//copy the character into the message
		msg.c = ch;

		msg.message_type = UART1_PRINT_SERVER_INPUT_MESSAGE;

		//send the message to the server
		//status = Send(tid, (char *) &msg, sizeof(msg), (char *) &rpl, sizeof(rpl));
		Send(tid, (char *) &msg, sizeof(msg), (char *) &rpl, sizeof(rpl));	
		
	} else if(channel == COM2) {		
		//we are sending to terminal, get the tid
		tid = WhoIs("u2send");
		
		//check the return value of WhoIs
		if(tid < 0){ return -1; }

		struct short_str_message msg;
		
		//copy the character into the message
		msg.str[0] = ch;
		msg.str[1] = '\0';
		copy_length = 2 + 4;
		
		msg.message_type = PRINT_SERVER_STRING_MESSAGE;
	
		//send the message to the server
		//status = Send(tid, (char *) &msg, copy_length, (char *) &rpl, sizeof(rpl));
		Send(tid, (char *) &msg, copy_length, (char *) &rpl, sizeof(rpl));
	} else {
		return -3; //NOT A PROPER COM CHANNEL
	}
	

	
	//TODO: add in checking for the return of the send and the reply
	//success
	return 0;
}

int Putbuff( int channel, struct print_buffer* buff ){
	int tid = -1;
	int copy_length = 0;
	struct extra_long_str_message msg;
	struct number_message rpl;
	
	if(channel == COM1){
		//we are sending to train, get the tid
		return -3;
		tid = WhoIs("u1send");
	}else if(channel == COM2){
		//we are sending to terminal, get the tid
		tid = WhoIs("u2send");
	}else{
		return -3; //NOT A PROPER COM CHANNEL
	}
	
	//check the return value of WhoIs
	if(tid < 0){ return -1; }
	
	//copy the buffer into the message
	copy_length = safestrcpy ( msg.str, buff->mem, CONFIG_EXTRA_LONG_STR_MSG_LENGTH );
	if(copy_length != buff->length){ return -4; }	//make sure we copied the whole buffer
	
	//calculate the length of the message to send
	copy_length = copy_length + 1 + 4;
	
	msg.message_type = PRINT_SERVER_STRING_MESSAGE;
	
	//send the message to the server
	//int status = Send(tid, (char *) &msg, copy_length, (char *) &rpl, sizeof(rpl));
	Send(tid, (char *) &msg, copy_length, (char *) &rpl, sizeof(rpl));
	
	//TODO: add in checking for the return of the send and the reply
	
	//success
	return 0;
}

void HijackCOM2(){
	//we are sending to terminal, get the tid
	int tid = WhoIs("u2send");

	//check the return value of WhoIs
	if(tid < 0){ return; }

	struct empty_message msg;

	msg.message_type = HIJACK_MESSAGE;

	Send(tid, (char *) &msg, sizeof(msg), (char *) 0, 0);
}
