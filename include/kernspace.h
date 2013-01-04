/*
	kernspace.h
	Include in all kernel space code
	Only includes other files
*/

#ifndef __KERNSPACE__H__
#define __KERNSPACE__H__

#include <kern/kern_globals.h>
#include <kern/tdlib.h>
#include <kern/scheduler.h>
#include <kern/syscall_handlers.h>
#include <kern/interrupt_handlers.h>

#include <kern/baseaddr.h>
#include <kern/kern_init.h>
#include <kern/kerexit.h>
#include <kern/handle_request.h>

//all headers common to user and kernel
#include <commonspace.h>

#endif


