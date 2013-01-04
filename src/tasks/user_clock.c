#include <userspace.h>


#define CLOCK_UPDATE_NONE 0
#define CLOCK_UPDATE_TSEC 1
#define CLOCK_UPDATE_SEC 2
#define CLOCK_UPDATE_MIN 3

void clock_redraw(struct print_buffer* pbuff, int minutes, int seconds, int tseconds);

void task_user_clock() {
	
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );
	
	clock_redraw(&pbuff, 0, 0, 0);
	
	int ticks_per_tsec = 10;
	
	int initial_time = Time();
	int time = initial_time;
	int next_time = time + ticks_per_tsec;
	
	while(1){
		//get the current time
		time = Time();

		if(time > next_time){
			
			next_time = time + ticks_per_tsec;
			int minutes = (time - initial_time) / 6000;
			int seconds = (time % 6000) / 100;
			int tseconds = ((time % 6000) % 100) / 10;
			clock_redraw(&pbuff, minutes, seconds, tseconds);
			
		}
		
		DelayUntil(next_time);
	}
}


void clock_redraw(struct print_buffer* pbuff, int minutes, int seconds, int tseconds){
	//Clear the screen
	ap_init_buff( pbuff );
	ap_putstr( pbuff, "\x1B[?25l");	//hide cursor
	ap_putstr( pbuff, "\x1B[1;51f");	//move to clock position
	ap_putstr( pbuff, "\x1B[1;32m");	//set attributes
	ap_printf( pbuff, "%d:%d:%d ", minutes, seconds, tseconds);
	ap_putstr( pbuff, "\x1B[0m");	//clear attributes
	ap_putstr( pbuff, "\x1B[u");	//return to CL 
	ap_putstr( pbuff, "\x1B[?25h");	//show cursor
	Putbuff( COM2, pbuff ); 
}

