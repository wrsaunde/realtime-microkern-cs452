#ifndef __REQUEST_H__
#define __REQUEST_H__

//function calls without parameters may use the default request struct

struct request {
	//must be included in all request structures
	int req_num;
	int retval;
	//end of required fields
};

struct request_create {
	//must be included in all request structures
	int req_num;
	int retval;
	//end of required fields
	
	int priority;
	int code;
};

struct request_send {
	//must be included in all request structures
	int req_num;
	int retval;
	//end of required fields
	
	int Tid;
	char *msg;
	int msglen;
	char *reply;
	int replylen;
};

struct request_receive {
	//must be included in all request structures
	int req_num;
	int retval;
	//end of required fields
	
	int *tid;
	char *msg;
	int msglen;
};

struct request_reply {
	//must be included in all request structures
	int req_num;
	int retval;
	//end of required fields
	
	int tid;
	char *reply;
	int replylen;
};

struct request_await_event {
	//must be included in all request structures
	int req_num;
	int retval;
	//end of required fields
	
	int eventid;
	char *event;
	int eventlen;
};


#endif

