/**
 *
 * kernel.c
 *
 **/
 
#include <kernspace.h>


int main( int argc, char* argv[] ) {	
	//test_system_idle_instrumented();
	//bwprintf(COM2, "Starting Kernel!");
	
	//bwsetfifo(COM2,OFF);
	
	//create the globals structure
	struct kern_globals GLOBAL;
	struct request* req;

	//2385260
	//0x2E56C

	/*
	bwprintf(COM2, "TID[%d]\tPC[%d]\tPRIOR[%d]\n\n", GLOBAL.ACTIVE_TASK->task_id, GLOBAL.ACTIVE_TASK->pc, GLOBAL.ACTIVE_TASK->priority);
	bwprintf(COM2, "REQ_NUM[%d]\n", GLOBAL.ACTIVE_TASK->req->req_num);
	bwprintf(COM2, "SND_TID[%d]\n", ((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid);
	bwprintf(COM2, "\tSND_STATE[%d]\n", GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].td_state);
	bwprintf(COM2, "\tSND_PRIOR[%d]\n", GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].priority);
	bwprintf(COM2, "\tSND_PC[%x]\n", GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].pc);
	bwprintf(COM2, "\tSND_REQ_NUM[%d]\n", GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].req->req_num);
	bwprintf(COM2, "\t\tSND_REQ_MSG[%x]\n", ((struct request_receive*)GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].req)->msg);
	bwprintf(COM2, "\t\tSND_REQ_MSG_LEN[%d]\n", ((struct request_receive*)GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].req)->msglen);
	bwprintf(COM2, "\t\tSND_REQ_NUM[%d]\n", ((struct request_receive*)GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].req)->req_num);
	bwprintf(COM2, "\t\tSND_REQ_TID_POINTER[%x]\n", ((struct request_receive*)GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].req)->tid);


	bwprintf(COM2, "ROUTE_SRV_TID[45]\n", GLOBAL.USER_TDS[((struct request_send*)GLOBAL.ACTIVE_TASK->req)->Tid].req->req_num);
	bwprintf(COM2, "\tROUTE_SRV_STACK_BOTTOM[%x]\n", GLOBAL.USER_TDS[45].stack_space_bottom);
	bwprintf(COM2, "\tROUTE_SRV_STACK_TOP[%x]\n", GLOBAL.USER_TDS[45].stack_space_top);
	bwprintf(COM2, "\tROUTE_SRV_STACK_POINTER[%x]\n", GLOBAL.USER_TDS[45].sp);
	//bwprintf(COM2, "\tROUTE_SRV_ROUTE_STRUCT_SIZE[%x]\n");

	bwprintf(COM2, "MSG_POINTER[%x]\n", ((struct request_send*)GLOBAL.ACTIVE_TASK->req)->msg);
	bwprintf(COM2, "\tMSG[0] = [%d]\n", ((int*)((struct request_send*)GLOBAL.ACTIVE_TASK->req)->msg)[0]);
	bwprintf(COM2, "\tMSG[1] = [%d]\n", ((int*)((struct request_send*)GLOBAL.ACTIVE_TASK->req)->msg)[1]);
	bwprintf(COM2, "\tMSG_LEN[%d]\n", ((struct request_send*)GLOBAL.ACTIVE_TASK->req)->msglen);

	bwprintf(COM2, "RPL_POINTER[%d]\n", ((struct request_send*)GLOBAL.ACTIVE_TASK->req)->reply);
	bwprintf(COM2, "\tRPL_LEN[%d]\n", ((struct request_send*)GLOBAL.ACTIVE_TASK->req)->replylen);
	bwgetc(COM2);
	 */

	//loop through all of the tds and find the task id with the greatest
	//number of blocked tasks on it, along with its task id
	/*
	int tdindex = 0;
	int block_max = 0;
	int i = 0;
	for(i = 0; i < TID_INDEX_MASK + 1; i++){
		int sqs = GLOBAL.USER_TDS[i].send_Q_size;
		if(sqs > block_max){
			block_max = sqs;
			tdindex = i;
		}
	}*/

	//bwprintf(COM2, "\n\nBLK[%d] TD_IND[%d] TID[%d] PRIOR[%d]\n\n", block_max, tdindex, GLOBAL.USER_TDS[tdindex].task_id, GLOBAL.USER_TDS[tdindex].priority);
	//bwgetc(COM2);
	
	initialize_globals(&GLOBAL);
	initialize_first_task(&GLOBAL);
	
	initialize_hardware();
	
	while(1){
		req = NULL;
		if(GLOBAL.SCHEDULER.highest_priority == -1){ return 0; } //NO TASKS
		GLOBAL.ACTIVE_TASK = schedule(&GLOBAL);
		req = kerexit(GLOBAL.ACTIVE_TASK);
		//check for quit
		if(req->req_num == SYSCALL_QUIT){ return 0; }
		kernel_assert(req != NULL, "req != NULL")
		handle_request(&GLOBAL, req);
	}
	
	bwprintf(COM2, "GoodBye World\n\n");
	
	cleanup_hardware();
	return 0;

}
