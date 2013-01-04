/**
 *
 * syscall_handlers.c
 *
 **/
 
#include <kernspace.h>

void requeue_active(struct kern_globals* GLOBAL){
	//just requeue the active task for now
	set_ready(GLOBAL, GLOBAL->ACTIVE_TASK);
	GLOBAL->ACTIVE_TASK = NULL;
}

void handle_my_tid(struct kern_globals* GLOBAL, struct request* req){
	req->retval = GLOBAL->ACTIVE_TASK->task_id;
}

void handle_kern_pointer(struct kern_globals* GLOBAL, struct request* req){
	req->retval = (int) GLOBAL;
}

void handle_my_parent_tid(struct kern_globals* GLOBAL, struct request* req){
	if(GLOBAL->ACTIVE_TASK->parent == NULL) {
		req->retval = -1;
	} else {
		req->retval = GLOBAL->ACTIVE_TASK->parent->task_id;
	}
}


void handle_create(struct kern_globals* GLOBAL, struct request_create* req){
	int priority = req->priority;
	void* func_point = (void *) req->code;
	if(priority >= 0 && priority < PRIORITY_NUM_PRIORITIES){
		struct task_descriptor* new_td = new_task(GLOBAL, func_point, priority);
		//set the parent
		new_td->parent = GLOBAL->ACTIVE_TASK;
		set_ready(GLOBAL, new_td);
		req->retval = new_td->task_id;
	}else{
		req->retval = -1;
	}
}

void handle_exit(struct kern_globals* GLOBAL, struct request* req){
	//update the tid
	GLOBAL->ACTIVE_TASK->td_state = TASK_ZOMBIE;
	
	int task_id = GLOBAL->ACTIVE_TASK->task_id;
	
	int new_tid = (((~TID_INDEX_MASK) & task_id) + TID_INDEX_MASK + 1) + (TID_INDEX_MASK & task_id);
	if(new_tid <= task_id){ return; }	//we have reached the limit of this task descriptors tids, dont add to free list
	
	GLOBAL->ACTIVE_TASK->task_id = new_tid;
	
	//add to the free list
	GLOBAL->ACTIVE_TASK->next_task = GLOBAL->free_list;
	GLOBAL->free_list = GLOBAL->ACTIVE_TASK;
}

/*
 *
 * SENDER ALWAYS REFERS TO THE TASK WHICH CALLED SEND
 *
 */


void execute_send_copy(struct kern_globals* GLOBAL, struct task_descriptor* sender, struct task_descriptor* receiver){
	struct request_send* sender_req = (struct request_send*) sender->req;
	struct request_receive* receiver_req = (struct request_receive*) receiver->req;
	
	//TODO: CHECK ALL MESSAGE BOUNDS AND MEMORY ADDRESSES
	//IF AN ERROR OCCURS RETURN -3 FOR SENDER
	if(0){
		sender_req->retval = -3;
		receiver_req->retval = 0;
		set_ready(GLOBAL, sender);
		set_ready(GLOBAL, receiver);
		return;
	}
	//set the tid for the receiver
	*(receiver_req->tid) = sender->task_id;
	
	int copy_length = sender_req->msglen;
	//get the minimum buffer size, and only copy that many bytes
	if(copy_length > receiver_req->msglen){ copy_length = receiver_req->msglen; }
	
	//set the receiver return value
	receiver_req->retval = copy_length;
		
	//everything should be ready, execute the message copy
	mem_copy((char*) sender_req->msg, (char*) receiver_req->msg, copy_length);
	
	//set the sender to be reply blocked
	sender->td_state = TASK_REPL_BLOCKED;

	//update the blocked task count
	receiver->send_Q_size--;
	
	//set the receiver to ready
	set_ready(GLOBAL, receiver);
}

void execute_reply_copy(struct kern_globals* GLOBAL, struct task_descriptor* sender, struct task_descriptor* replier){
	struct request_send* sender_req = (struct request_send*) sender->req;
	struct request_reply* replier_req = (struct request_reply*) replier->req;
	
	//TODO: CHECK FOR MEMORY ERRORS (MEM PROTECTION)
	if(0){
		sender_req->retval = -3;
		replier_req->retval = -4;
		set_ready(GLOBAL, sender);
		set_ready(GLOBAL, replier);
		return;
	}

	
	//set the replier return value
	replier_req->retval = 0;
	//set the sender return value
	sender_req->retval = replier_req->replylen;
	
	int copy_length = sender_req->replylen;
	//get the minimum buffer size, and only copy that many bytes
	if(copy_length > replier_req->replylen){ copy_length = replier_req->replylen; }
	
	//everything should be ready, execute the message copy
	mem_copy((char*) replier_req->reply, (char*) sender_req->reply, copy_length);

	//bwprintf(COM2, "execute_reply_copy sender_req->retval: %d\r\n", sender_req->retval);
	
	
	//set the sender to be ready, it must come before replier
	set_ready(GLOBAL, sender);
	//set the receiver to ready
	set_ready(GLOBAL, replier);
	
}

void handle_send(struct kern_globals* GLOBAL, struct request_send* req){
	
	//check if the tid is impossible
	if(!is_tid_valid(req->Tid)){
		req->retval = -1;
		requeue_active(GLOBAL);
		return;
	}
	
	//check if the task exists
	if(!is_tid_in_use(GLOBAL, req->Tid)){
		req->retval = -2;
		requeue_active(GLOBAL);
		return;
	}
	
	//get the td of the target
	int td_index = extract_td_index(req->Tid);
	struct task_descriptor* td_receiver = &GLOBAL->USER_TDS[td_index];
	
	//set the task to RECV_BLOCKED
	GLOBAL->ACTIVE_TASK->td_state = TASK_RECV_BLOCKED;
	//add the request
	GLOBAL->ACTIVE_TASK->req = (struct request*)req;

	td_receiver->send_Q_size++;
	
	if(td_receiver->td_state == TASK_SEND_BLOCKED){
		//the task is waiting, just execute the copy
		execute_send_copy(GLOBAL, GLOBAL->ACTIVE_TASK, td_receiver);
	}else{
		//the target hasn't called recv, add to the queue
		if(td_receiver->first_sender == NULL){
			td_receiver->first_sender = GLOBAL->ACTIVE_TASK;
			td_receiver->end_sender = GLOBAL->ACTIVE_TASK;
			GLOBAL->ACTIVE_TASK->next_task = NULL;
		}else{
			td_receiver->end_sender->next_task = GLOBAL->ACTIVE_TASK;
			td_receiver->end_sender = GLOBAL->ACTIVE_TASK;
		}
	}
}

void handle_receive(struct kern_globals* GLOBAL, struct request_receive* req){
	GLOBAL->ACTIVE_TASK->td_state = TASK_SEND_BLOCKED;
	GLOBAL->ACTIVE_TASK->req = (struct request*)req;
	
	//if there are no senders waiting, block
	if(GLOBAL->ACTIVE_TASK->first_sender == NULL){
		return;
	}
	
	//we have a sender, pop the td and execute the send
	struct task_descriptor* sender = GLOBAL->ACTIVE_TASK->first_sender;
	GLOBAL->ACTIVE_TASK->first_sender = sender->next_task;
	if(sender->next_task == NULL){
		GLOBAL->ACTIVE_TASK->end_sender = NULL;
	}
	
	execute_send_copy(GLOBAL, sender, GLOBAL->ACTIVE_TASK);
}

void handle_reply(struct kern_globals* GLOBAL, struct request_reply* req){

	//check if the tid is impossible
	if(!is_tid_valid(req->tid)){
		req->retval = -1;
		requeue_active(GLOBAL);
		return;
	}
	
	//check if the task exists
	if(!is_tid_in_use(GLOBAL, req->tid)){
		req->retval = -2;
		requeue_active(GLOBAL);
		return;
	}
	
	//get the td of the target
	int td_index = extract_td_index(req->tid);
	struct task_descriptor* sender = &GLOBAL->USER_TDS[td_index];
	
	//check if the task is reply blocked
	if(sender->td_state != TASK_REPL_BLOCKED){
		req->retval = -3;
		requeue_active(GLOBAL);
		return;
	}
	
	//add the request
	GLOBAL->ACTIVE_TASK->req = (struct request*)req;
	
	//execute the reply
	execute_reply_copy(GLOBAL, sender, GLOBAL->ACTIVE_TASK);

}

void handle_await_event(struct kern_globals* GLOBAL, struct request_await_event* req){
	//check if the event id is invalid
	if(req->eventid >= EVENTS_NUM_EVENTS || req->eventid < 0){
		//return the invalid code, and requeue the task
		req->retval = EVENT_RETURN_INVALID_EVENT;
		requeue_active(GLOBAL);
		return;
	}
	
	//make sure there isnt already a task waiting on the event
	if(GLOBAL->EVENT[req->eventid].td != NULL){
		req->retval = EVENT_RETURN_TASK_ALREADY_WAITING;
		requeue_active(GLOBAL);
		return;
	}
	
	//save the request structure, and queue the event
	GLOBAL->ACTIVE_TASK->req = (struct request*)req;
	GLOBAL->EVENT[req->eventid].td = GLOBAL->ACTIVE_TASK;
	
	//if we are the terminal input, and its not empty
	if(req->eventid == EVENT_UART2_RECEIVE_READY && GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].cbuff.state != CBUFF_EMPTY) {
		struct request_await_event* uart2_req = (struct request_await_event*) GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].td->req;
		
		//set the return value
		uart2_req->retval = 0;

		int i = 0;
		while ( GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].cbuff.state != CBUFF_EMPTY && i < 15){
			uart2_req->event[i] = cbuffer_pop( &(GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].cbuff));
			i++;
		}
		req->event[i] = '\0';
		
		//ready the task
		set_ready(GLOBAL, GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].td);
		
		//clear the event slot
		GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].td = NULL;
	}
	
	//if we are the clock tick, and its not empty
	if(req->eventid == EVENT_CLOCK_TICK && GLOBAL->EVENT[EVENT_CLOCK_TICK].cbuff.state != CBUFF_EMPTY) {
		struct request_await_event* clock_req = (struct request_await_event*) GLOBAL->EVENT[EVENT_CLOCK_TICK].td->req;
		
		//set the return value
		clock_req->retval = 0;

		int i = 0;
		while ( GLOBAL->EVENT[EVENT_CLOCK_TICK].cbuff.state != CBUFF_EMPTY && i < 15){
			clock_req->event[i] = cbuffer_pop( &(GLOBAL->EVENT[EVENT_CLOCK_TICK].cbuff));
			i++;
		}
		req->event[i] = '\0';
		
		//ready the task
		set_ready(GLOBAL, GLOBAL->EVENT[EVENT_CLOCK_TICK].td);
		
		//clear the event slot
		GLOBAL->EVENT[EVENT_CLOCK_TICK].td = NULL;
	}
	
	if(req->eventid == EVENT_UART2_SEND_READY) {
		//Re-activate the UART2 interrupt
		int * ctlr, buf;
		ctlr = (int *)( UART2_BASE + UART_CTLR_OFFSET );
		buf = *ctlr;
		*ctlr = buf | TIEN_MASK;
	}
	
	if(req->eventid == EVENT_UART1_SEND_READY_INITIAL) {
		//Re-activate the UART1 transmit and modem status interrupts
		int * ctlr, buf;
		ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
		buf = *ctlr;
		*ctlr = buf | TIEN_MASK | MSIEN_MASK;
	} else if(req->eventid == EVENT_UART1_SEND_READY) {
		//Re-activate the UART1 modem status interrupt
		int * ctlr, buf;
		ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
		buf = *ctlr;
		*ctlr = buf | MSIEN_MASK;		
	}
}


