#include <userspace.h>

//helper functions
void debug_kernel_output( volatile struct kern_globals* KERN );
//end of helpers


#define KERN_TAB_DISPLAY_REFRESH_RATE 10


void task_user_kern_debug_tab( ) {

	Delay( 10 );

	//messaging stuff
	int recv_tid = 0;
	struct two_number_message msg;

	//tab stuff
	int tab_show = FALSE;
	WorkerPeriodicDelay( KERN_TAB_DISPLAY_REFRESH_RATE );


	//get a pointer to the kernel data structure
	volatile struct kern_globals* KERN = (struct kern_globals*)KernGlobalsPointer( );

	//register as a tab
	TabRegister( "KERN" );

	while( 1 ) {
		//get another message, we have reached the end
		Receive( &recv_tid, (char *)&msg, sizeof (msg) );

		//check message type and train tid
		switch( msg.message_type ) {
			case DELAY_WORKER_MESSAGE:
				//send the delay worker away
				Reply( recv_tid, (char *)0, 0 );

				//update each entry and write it
				if(tab_show){
					debug_kernel_output( KERN );
				}
				break;
			case TAB_DISABLE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				tab_show = FALSE;
				break;
			case TAB_ENABLE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				tab_show = TRUE;
				clear_tab_space( );
				debug_kernel_output( KERN );
				break;
			default:
				Reply( recv_tid, (char *)0, 0 );
				break;
		}
	}

	Exit( );
}


void debug_kernel_output( volatile struct kern_globals* KERN ) {

	int next_tid = KERN->next_task_id;
	volatile struct task_descriptor* td = KERN->ACTIVE_TASK;
	int active_task = td->task_id;

	//loop through all of the tds and find the task id with the greatest
	//number of blocked tasks on it, along with its task id
	int tid = 0;
	int block_max = 0;
	int i = 0;
	for(i = 0; i < TID_INDEX_MASK + 1; i++){
		volatile struct task_descriptor* utd = &KERN->USER_TDS[i];
		int sqs = utd->send_Q_size;
		if(sqs > block_max){
			block_max = sqs;
			tid = i;
		}
	}

	//Printf( COM2, "\x1B[7;29fNEXT TID [%d]\x1B[8;29fACTIVE TASK [%d]", next_tid , active_task );
	Printf( COM2, "\x1B[9;29f               \x1B[9;29fTID[%d]\x1B[10;29f\x1B[10;29f                     \x1B[10;29fBLK[%d]", tid , block_max );
}
