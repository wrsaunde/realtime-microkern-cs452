/**
 *
 * event.h
 *
 **/

#ifndef __EVENT__H__
#define __EVENT__

#define EVENTS_NUM_EVENTS 6
#define EVENTS_MAX_DATA_SIZE 1

//System call return values
#define EVENT_RETURN_INVALID_EVENT -1
#define EVENT_RETURN_CORRUPT_DATA -2
#define EVENT_RETURN_TASK_ALREADY_WAITING -4


// List of events, when adding you must update num_events
// and possibly max_data_size if the event uses a larger
// structure

#define EVENT_CLOCK_TICK 0 
#define EVENT_UART1_SEND_READY 1 
#define EVENT_UART1_RECEIVE_READY 2 
#define EVENT_UART2_SEND_READY 3 
#define EVENT_UART2_RECEIVE_READY 4 
#define EVENT_UART1_SEND_READY_INITIAL 5 

#endif
