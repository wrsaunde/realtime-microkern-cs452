/*
	commonspace.h
	Includes for both kernel and userspace
	Only includes other files
*/

#ifndef __COMMONSPACE__H__
#define __COMMONSPACE__H__

//all user task declarations
#include <tasks/all.h>

//all libraries
#include <lib/all.h>

//hardware information
#include <config/ts7200.h>

//train information
#include <config/train_constants.h>

//events
#include <common/event.h>

//requests
#include <common/request.h>

//debug functions
#include <common/debug.h>

//syscall codes
#include <sys_calls/sys_call_codes.h>

//Keywords such as NULL
#include <config/key_words.h>

//Priorities
#include <common/priority.h>

//character codes
#include <config/char_codes.h>

//Kernel configuration (needed for shared memory operations)
#include <config/kern.h>

#endif


