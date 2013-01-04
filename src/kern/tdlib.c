/**
 *
 * tdlib.c
 *
 **/
#include <kernspace.h>



int extract_td_index(int Tid){
	return (TID_INDEX_MASK & Tid);
}

int is_tid_valid(int Tid){
	if(Tid >= 0){ return 1; }
	return 0;
}

int is_tid_in_use(struct kern_globals* GLOBAL, int Tid){
	//check if we have a tid higher than the max
	if(GLOBAL->next_free_td <= TID_INDEX_MASK && Tid >= GLOBAL->next_free_td){ return 0; }
	
	//check the task corresponding td and its Tid
	struct task_descriptor* td = &GLOBAL->USER_TDS[extract_td_index(Tid)];
	
	if(td->task_id != Tid || td->td_state == TASK_ZOMBIE){ return 0; }
	
	//this tid is valid and in use
	return 1; 
}

struct task_descriptor* new_task(struct kern_globals* GLOBAL, void (*code) (), int priority){
	//first check if there is a free td in the array
	int td_index = 0;		//index in td array
	
	struct task_descriptor* td = NULL;

	
	if(GLOBAL->next_free_td <= TID_INDEX_MASK){
		td_index = GLOBAL->next_free_td++;
		td = &(GLOBAL->USER_TDS[td_index]);
		/* INITIALIZE THE TD TO DEFAULTS */
		td->sp = 0;
		td->pc = 0;
		td->CPSR = 0x50;
	
		td->td_state = TASK_INITIALIZED;
		td->priority = 0;
		td->task_id = td_index;
		td->parent = NULL;	
		td->next_task = NULL;
		td->req = NULL;
	
		td->first_sender = NULL;
		td->end_sender = NULL;
		/* END TD INITIALIZATION */
	}else{
		kernel_assert(GLOBAL->free_list != NULL,"GLOBAL->free_list != NULL");
		
		//get the td off the free list
		td = GLOBAL->free_list;
		GLOBAL->free_list = GLOBAL->free_list->next_task;
		td->next_task = NULL;
		
		/* INITIALIZE THE TD TO DEFAULTS */
		td->sp = 0;
		td->pc = 0;
		td->CPSR = 0x50;
	
		td->td_state = TASK_INITIALIZED;
		td->priority = 0;
		//dont overwrite the task id, its updated in exit
		//td->task_id = 0;
		
		td->parent = NULL;	
		td->next_task = NULL;
		td->req = NULL;
	
		td->first_sender = NULL;
		td->end_sender = NULL;
		/* END TD INITIALIZATION */
	}
	
	//set the stack pointer and zero fill current stack frame
	td->sp = (char*) td->stack_space_top;
	int i = 0;
	for(i = 1;i <= 16; i++){
		td->sp = (char*) td->sp - 4;
		*(td->sp) = 0;
	}
	
	//set the pc value
	td->pc = (char*)((int)code + GLOBAL->load_offset);

	//set the send Q size
	td->send_Q_size = 0;
	
	//set the priority
	td->priority = priority;
	
	return td;
}

