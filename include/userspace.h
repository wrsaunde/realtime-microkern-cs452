/*
	userspace.h
	Include in all user space code (tasks etc.)
	Only includes other files
*/

#ifndef __USERSPACE__H__
#define __USERSPACE__H__


//system call entry
#include <sys_calls/syscall_entry.h>

//used for debugging with kern_pointer sys call
#include <kern/kern_globals.h>


//all system call declarations
#include <sys_calls/sys_calls.h>

//messages
#include <tasks/message.h>

//configuration
#include <config/user.h>
#include <config/game.h>

//all headers common to user and kernel
#include <commonspace.h>

#include <tasks/track_data.h>
#include <tasks/track_node.h>

#include <tasks/train_data.h>

#endif


