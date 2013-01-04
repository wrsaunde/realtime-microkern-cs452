/**
 *
 * sys_call_codes.h
 *
 **/

#ifndef __SYS_CALL_CODES__H__
#define __SYS_CALL_CODES__H__

//Do NOT change IRQ!!!! It's magic!
#define IRQ 7

#define SYSCALL_PASS 10
#define SYSCALL_EXIT 11
#define SYSCALL_CREATE 12
#define SYSCALL_MY_TID 13
#define SYSCALL_MY_PARENT_TID 14
#define SYSCALL_SEND 15
#define SYSCALL_RECEIVE 16
#define SYSCALL_REPLY 17
#define SYSCALL_AWAIT_EVENT 18

#define SYSCALL_TIME 19

#define SYSCALL_QUIT 20
#define SYSCALL_KERN_POINTER 21


//Return valuse from nameserver
#define NAME_NOT_FOUND -3
#define NAME_DUPLICATE -4

#endif
