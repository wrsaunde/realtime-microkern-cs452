#include <kernspace.h>

int highest_non_empty_prior(struct kern_globals* GLOBAL, int start);

void initialize_scheduler(struct kern_globals* GLOBAL){
	GLOBAL->SCHEDULER.highest_priority = -1;
	
	int i = 0;
	for(i = 0;i < PRIORITY_NUM_PRIORITIES; i++){
		GLOBAL->SCHEDULER.ready_queues[i].front = NULL;
		GLOBAL->SCHEDULER.ready_queues[i].end = NULL;
	}
}

struct task_descriptor* schedule(struct kern_globals* GLOBAL){
	int next_prior = GLOBAL->SCHEDULER.highest_priority;
	if(next_prior == -1){
		//there are no ready tasks, we should exit kernel
	}
	
	//get the next task to run
	struct task_descriptor* next = GLOBAL->SCHEDULER.ready_queues[next_prior].front;
	
	//remove the current task from the ready queue
	GLOBAL->SCHEDULER.ready_queues[next_prior].front = next->next_task;
	next->next_task = NULL;
	
	if(GLOBAL->SCHEDULER.ready_queues[next_prior].front == NULL){
		//set the back to also be NULL
		GLOBAL->SCHEDULER.ready_queues[next_prior].end = NULL;
		
		//this priority is empty, find the next highest which is not empty
		GLOBAL->SCHEDULER.highest_priority = highest_non_empty_prior( GLOBAL, next_prior );
	}
	//set up the td to run
	next->td_state = TASK_ACTIVE;
	//return the task desciptor
	return next;
}

void set_ready(struct kern_globals* GLOBAL, struct task_descriptor* td){
	
	//make sure we update the pointers so that we dont queue send blocked tasks
	td->next_task = NULL;
	
	//if we are already ready, do nothing
	if(td->td_state == TASK_READY){ return; }
	
	int priority = td->priority;
	//if we are a higher priority, set it.
	if(priority > GLOBAL->SCHEDULER.highest_priority){
		GLOBAL->SCHEDULER.highest_priority = priority;
	}
	if(GLOBAL->SCHEDULER.ready_queues[priority].front == NULL){
		GLOBAL->SCHEDULER.ready_queues[priority].front = td;
		GLOBAL->SCHEDULER.ready_queues[priority].end = td;
	}else{
		GLOBAL->SCHEDULER.ready_queues[priority].end->next_task = td;
		GLOBAL->SCHEDULER.ready_queues[priority].end = td;
	}
	
	//set the task information
	td->req = NULL;
	td->td_state = TASK_READY;
}

int highest_non_empty_prior(struct kern_globals* GLOBAL, int start){
	int i = start;
	
	while(i >= 0){
		if(GLOBAL->SCHEDULER.ready_queues[i].front != NULL){
			return i;
		}
		i--;
	}
	return -1;
}
