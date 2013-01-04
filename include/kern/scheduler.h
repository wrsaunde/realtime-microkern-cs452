/**
 *
 * scheduler.h
 *
 **/
 
#ifndef __SCHEDULER__H__
#define __SCHEDULER__H__
 
//Required Modules
#include <config/kern.h>
#include <kern/kern_globals.h>

struct task_descriptor* schedule(struct kern_globals* GLOBAL);
void set_ready(struct kern_globals* GLOBAL, struct task_descriptor* td);
void initialize_scheduler(struct kern_globals* GLOBAL);
#endif

