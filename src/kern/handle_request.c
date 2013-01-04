#include <kernspace.h>

void handle_request(struct kern_globals* GLOBAL, struct request* req) {
	//bwprintf(COM2, "SYSCALL: %d\n", req->req_num);
	switch(req->req_num) {
		case IRQ:
			//requeue the task that was interrupted
			requeue_active(GLOBAL);
			//handle interrupts
			handle_interrupts(GLOBAL);
			break;
		case SYSCALL_MY_TID:
			handle_my_tid(GLOBAL, req);
			requeue_active(GLOBAL);
			break;
		case SYSCALL_MY_PARENT_TID:
			handle_my_parent_tid(GLOBAL, req);
			requeue_active(GLOBAL);
			break;
		case SYSCALL_CREATE:
			handle_create(GLOBAL,(struct request_create*) req);
			requeue_active(GLOBAL);
			break;
		case SYSCALL_SEND:
			handle_send(GLOBAL,(struct request_send*) req);
			break;
		case SYSCALL_RECEIVE:
			handle_receive(GLOBAL,(struct request_receive*) req);
			break;
		case SYSCALL_REPLY:
			handle_reply(GLOBAL,(struct request_reply*) req);
			requeue_active(GLOBAL);
			break;
		case SYSCALL_AWAIT_EVENT:
			handle_await_event(GLOBAL,(struct request_await_event*) req);
			break;
		case SYSCALL_EXIT:
			handle_exit(GLOBAL, req);;
			break;
		case SYSCALL_KERN_POINTER:
			handle_kern_pointer(GLOBAL, req);
			requeue_active(GLOBAL);
			break;
		case SYSCALL_QUIT:
			//kill the kernel
			//should be caught already and should never get here
			break;
		default:
			//undefined system call code, just return -1
			req->req_num = -1;
			requeue_active(GLOBAL);
			break;
	}
}
