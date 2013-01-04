/**
 *
 * syscall.c
 *
 **/

#include <userspace.h>

void Pass() {
	struct request req;
	req.req_num = SYSCALL_PASS;
	syscall_entry(&req);
}

void Quit() {
	struct request req;
	req.req_num = SYSCALL_QUIT;
	syscall_entry(&req);
}

void Exit() {
	struct request req;
	req.req_num = SYSCALL_EXIT;
	syscall_entry(&req);	
}

int Create(int priority, void(*code)) {
	struct request_create req;
	req.req_num = SYSCALL_CREATE;
	req.priority = priority;
	req.code = (int) code;
	syscall_entry((struct request*)&req);
	return req.retval;
}

int MyTid() {
	struct request req;
	req.req_num = SYSCALL_MY_TID;
	syscall_entry(&req);
	return req.retval;	
}

int KernGlobalsPointer() {
	struct request req;
	req.req_num = SYSCALL_KERN_POINTER;
	syscall_entry(&req);
	return req.retval;
}

int MyParentTid() {
	struct request req;
	req.req_num = SYSCALL_MY_PARENT_TID;
	syscall_entry(&req);
	return req.retval;	
}

int Send( int Tid, char *msg, int msglen, char *reply, int replylen ) {
	struct request_send req;
	req.req_num = SYSCALL_SEND;
	
	req.Tid = Tid;
	req.msg = msg;
	req.msglen = msglen;
	req.reply = reply;
	req.replylen = replylen;
	
	//req.retval = 42;
	
	syscall_entry((struct request*)&req);

	//bwprintf(COM2, "Send req->retval: %d\r\n", req.retval);
	
	return req.retval;
}

int Receive( int *tid, char *msg, int msglen ) {
	struct request_receive req;
	req.req_num = SYSCALL_RECEIVE;
	
	req.tid = tid;
	req.msg = msg;
	req.msglen = msglen;
	
	syscall_entry((struct request*)&req);
	return req.retval;
}

int Reply( int tid, char *reply, int replylen ) {
	struct request_reply req;
	req.req_num = SYSCALL_REPLY;
	
	req.tid = tid;
	req.reply = reply;
	req.replylen = replylen;
	
	syscall_entry((struct request*)&req);
	return req.retval;
}

int AwaitEvent( int eventid, char *event, int eventlen ){
	struct request_await_event req;
	req.req_num = SYSCALL_AWAIT_EVENT;
	
	req.eventid = eventid;
	req.event = event;
	req.eventlen = eventlen;
	
	syscall_entry((struct request*)&req);
	return req.retval;
}

int RegisterAs( char * name ) {
	struct nameserver_message msg;
	struct number_message rpl;
	int status = 0;
	//RegisterAs message containing the task name
	msg.message_type = REGISTERAS_MESSAGE;
	safestrcpy(msg.name, name, CONFIG_NAMESRV_NAME_LENGTH);
	status = Send( CONFIG_NAMESRV_TID, (char*)&msg, sizeof(msg), (char*)&rpl, sizeof(rpl));
	if(status == sizeof(rpl)) {
		if(rpl.message_type == NUMBER_MESSAGE) {
			//Successful completion
			return rpl.num;
		} else {
			//Something went wrong (likely an invalid operation)
			//Report that we didn't message the nameserver
			return -2;
		}
	} else {
		//Something went wrong in message transmission
		//Report that we didn't have the right tid for the nameserver
		return -1;
	}
}


int WhoIs( char * name ) {
	struct nameserver_message msg;
	struct number_message rpl;
	int status = 0;
	//RegisterAs message containing the task name
	msg.message_type = WHOIS_MESSAGE;
	safestrcpy(msg.name, name, CONFIG_NAMESRV_NAME_LENGTH);
	status = Send( CONFIG_NAMESRV_TID, (char*)&msg, sizeof(msg), (char*)&rpl, sizeof(rpl));
	if(status == sizeof(rpl)) {
		if(rpl.message_type == NUMBER_MESSAGE) {
			//Successful completion, return the value we got back
			return rpl.num;
		} else {
			//Something went wrong (likely an invalid operation)
			//Report that we didn't message the nameserver
			return -2;
		}
	} else {
		//Something went wrong in message transmission
		//Report that we didn't have the right tid for the nameserver
		return -1;
	}
}

int Time() {
	struct empty_message msg;
	struct number_message rpl;
	int status = 0;
	int clock_serv = -1;
	clock_serv = WhoIs("clock_serv");
	//RegisterAs message containing the task name
	msg.message_type = CLOCK_TIME_MESSAGE;
	status = Send(clock_serv , (char*)&msg, sizeof(msg), (char*)&rpl, sizeof(rpl));
	if(status == sizeof(rpl)) {
		if(rpl.message_type == CLOCK_TIME_REPLY) {
			//Successful completion, return the value we got back
			return rpl.num;
		} else {
			//Something went wrong (likely an invalid operation)
			//Report that we didn't message the clockserver
			return -2;
		}
	} else {
		//Something went wrong in message transmission
		//Report that we didn't have the right tid for the clockserver
		return -1;
	}
}

int Delay(int delay) {
	struct number_message msg;
	struct number_message rpl;
	int status = 0;
	int clock_serv = -1;
	clock_serv = WhoIs("clock_serv");
	//RegisterAs message containing the task name
	msg.message_type = CLOCK_DELAY_MESSAGE;
	msg.num = delay;
	status = Send(clock_serv , (char*)&msg, sizeof(msg), (char*)&rpl, sizeof(rpl));
	if(status == sizeof(rpl)) {
		if(rpl.message_type == CLOCK_WAKEUP) {
			//Successful completion, return the value we got back
			return rpl.num;
		} else {
			//Something went wrong (likely an invalid operation)
			//Report that we didn't message the clockserver
			return -2;
		}
	} else {
		//Something went wrong in message transmission
		//Report that we didn't have the right tid for the clockserver
		return -1;
	}
}

int DelayUntil(int delay) {
	struct number_message msg;
	struct number_message rpl;
	int status = 0;
	int clock_serv = -1;
	clock_serv = WhoIs("clock_serv");
	//RegisterAs message containing the task name
	msg.message_type = CLOCK_DELAY_UNTIL_MESSAGE;
	msg.num = delay;
	status = Send(clock_serv , (char*)&msg, sizeof(msg), (char*)&rpl, sizeof(rpl));
	if(status == sizeof(rpl)) {
		if(rpl.message_type == CLOCK_WAKEUP) {
			//Successful completion, return the value we got back
			return rpl.num;
		} else {
			//Something went wrong (likely an invalid operation)
			//Report that we didn't message the clockserver
			return -2;
		}
	} else {
		//Something went wrong in message transmission
		//Report that we didn't have the right tid for the clockserver
		return -1;
	}
}

