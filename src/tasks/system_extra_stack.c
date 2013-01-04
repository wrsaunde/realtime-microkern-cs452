#include <userspace.h>

//NEVER USE THIS TASK!!! MWAHAHAHAHAH!!!!


void task_system_extra_stack( ) {

	int tid_pointer;
	//go to sleep forever, so that the task before me can use my stack space
	Receive( &tid_pointer, (char*)0, 0 );
	Exit( );
}