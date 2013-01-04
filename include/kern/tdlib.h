/**
 *
 * tdlib.h
 *
 **/

#ifndef __TDLIB__H__
#define __TDLIB__H__

//REQUIRED HEADERS
#include <config/kern.h>
#include <kern/kern_globals.h>

int extract_td_index(int Tid);

int is_tid_valid(int Tid);

int is_tid_in_use(struct kern_globals* GLOBAL, int Tid);

struct task_descriptor* new_task(struct kern_globals* GLOBAL, void (*code) (), int priority);

#endif

