/**
 *
 * kern_config.h
 *
 **/

#ifndef __KERN_CONFIG__H__
#define __KERN_CONFIG__H__

/*
 *
 * Kernel Configuration
 *
 **/

//size of a task stack in bytes
//#define STACK_SIZE 65536
#define STACK_SIZE 65536

#define MEMORY_SIZE 33554432
#define KERNEL_PADDING (1048576 * 3)

//max number of tasks
#define TID_INDEX_MASK 0x000000FF

//Length of a clock tick (remember to subtract 1 when setting it as the interval length of a timer)
#define TICK_INTERVAL 20 

//size of character buffer for events
#define EVENTS_CHAR_BUFFER_SIZE 2048

//Shared memory constants
#define SHM_START (MEMORY_SIZE - KERNEL_PADDING)

#define SHM_NAMES 0
#define SHM_NAMES_SIZE 1024

#define SHM_GOSSIP 1
#define SHM_GOSSIP_SIZE 4096

#define SHM_TOTAL_SIZE (SHM_NAMES_SIZE + SHM_GOSSIP_SIZE)

#endif

//Uncomment this line to turn off assertions
//#define NDEBUG

