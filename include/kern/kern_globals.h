/**
 *
 * kern_globals.h
 *
 **/

#ifndef __KERN_GLOBALS__H__
#define __KERN_GLOBALS__H__

#define NULL 0

//REQUIRED HEADERS
#include <config/kern.h>
#include <common/priority.h>
#include <common/event.h>
#include <lib/all.h>

typedef int cspr_val;

typedef int task_state;
#define TASK_INITIALIZED 0
#define TASK_ACTIVE 1
#define TASK_READY 2
#define TASK_ZOMBIE 3
#define TASK_RECV_BLOCKED 4
#define TASK_SEND_BLOCKED 5
#define TASK_REPL_BLOCKED 6

struct task_descriptor {
	//DO NOT CHANGE FIRST 3 VARIABLES, THEY ARE USED IN ASM
	//User Task State
	char* sp;		//saved value of the stack pointer
	cspr_val CPSR;
	char* pc;		//saved value of the program counter
	//END OF ASM DEPENDENT VARIABLES

	//Extra Kernel Information
	int td_state;		//current state of this td
	int priority;
	int task_id;		//id of the task
	char* stack_space_top;
	char* stack_space_bottom;
	struct task_descriptor* parent;	//parent of the task	
	struct task_descriptor* next_task;	//Next task in current queue
	struct request* req;	//points to the request which caused the task to block
	
	//Send Queue Info
	int send_Q_size;
	struct task_descriptor* first_sender;
	struct task_descriptor* end_sender;
};

struct td_queue {
	struct task_descriptor* front;
	struct task_descriptor* end;
};

struct scheduler_data {
	//what is the highest non-empty priority
	int highest_priority;
	//priority ready queues
	struct td_queue ready_queues[PRIORITY_NUM_PRIORITIES];
};

struct event{
	struct task_descriptor* td;
	
	//used if an event was triggered and no task was waiting
	int too_slow;
	char buff_mem[EVENTS_CHAR_BUFFER_SIZE];
	struct char_buffer cbuff;
};


struct kern_globals {
	//space for user task descriptors
	int next_free_td;
	struct task_descriptor USER_TDS[TID_INDEX_MASK + 1];
	
	struct task_descriptor* free_list;
	
	//data for the scheduler
	struct scheduler_data SCHEDULER;
	
	//the id of the active task
	struct task_descriptor* ACTIVE_TASK;
	
	struct event EVENT[EVENTS_NUM_EVENTS];
	
	//the address redboot has loaded the elf file
	int load_offset;
	
	int next_task_id;
};


void initialize_globals(struct kern_globals* GLOBAL);
#endif

