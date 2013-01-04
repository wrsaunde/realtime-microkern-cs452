/**
 *
 * syscall_handlers.h
 *
 **/

#ifndef __SYSCALL_HANDLERS__H__
#define __SYSCALL_HANDLERS__

//REQUIRED HEADERS
#include <kern/kern_globals.h>
#include <config/kern.h>
#include <common/request.h>

void requeue_active(struct kern_globals* GLOBAL);

void handle_my_tid(struct kern_globals* GLOBAL, struct request* req);
void handle_kern_pointer(struct kern_globals* GLOBAL, struct request* req);
void handle_my_parent_tid(struct kern_globals* GLOBAL, struct request* req);
void handle_create(struct kern_globals* GLOBAL, struct request_create* req);
void handle_exit(struct kern_globals* GLOBAL, struct request* req);

void handle_send(struct kern_globals* GLOBAL, struct request_send* req);
void handle_receive(struct kern_globals* GLOBAL, struct request_receive* req);
void handle_reply(struct kern_globals* GLOBAL, struct request_reply* req);

void handle_await_event(struct kern_globals* GLOBAL, struct request_await_event* req);

#endif

