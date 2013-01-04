#include <userspace.h>

void task_system_idle() {
	int i = 0;
	while(1) {		
		i++;
	}
	Exit();
}

void task_system_idle_instrumented() {
	int i = 0;
	int last_time = 0;
	int current_time = 0;
	int idle_time = 0;
	int total_time = 0;
	int last_output_time = 0;
	unsigned int volatile * timer4_low = (unsigned int volatile *) TIMER4_LOW;
	unsigned int volatile * timer4_high = (unsigned int volatile *) TIMER4_HIGH;
	int printed_last_time = 0;

	const int normal_loop_max_time = 20;
	const int print_interval = 500000;

	struct print_buffer pbuff;

	*timer4_high = 0xFFFFFFFF;//TIMER4_ENABLE_MASK;
	for(i = 0; i < 100; i++) {

	}
    i = 0;
	current_time = last_time = *timer4_low;

	while(1) {
		current_time = *timer4_low;

		if(printed_last_time == 1) {
			last_time = current_time;
			printed_last_time = 0;
		}

		if(current_time < last_time) {
			last_time = current_time;
			last_output_time = current_time;
			continue;
		}
		total_time += current_time - last_time;

		if(current_time <= last_time + normal_loop_max_time) {
			idle_time += current_time - last_time;
		}



		if(current_time > last_output_time + print_interval) {
			ap_init_buff( &pbuff );
			//ap_putstr( &pbuff, "\x1B[?25l");	//hide cursor
			ap_putstr( &pbuff, "\x1B[1;75f");	//move to idle% position
			//ap_putstr( &pbuff, "\x1B[1;32m");	//set attributes
			ap_printf( &pbuff, "%d%s", ( (100 * idle_time) / total_time), "%" );
			//ap_putstr( &pbuff, "\x1B[0m");		//clear attributes
			ap_putstr( &pbuff, "\x1B[u");		//return to CL
			//ap_putstr( &pbuff, "\x1B[?25h");	//show cursor
			Putbuff( COM2, &pbuff );

			if(i%4 == 0) {
				//CommandOutput("total_time: %d idle_time: %d current time %d", total_time, idle_time, current_time);
			}

			last_output_time = current_time;
			total_time = 0;
			idle_time = 0;
			i++;
			printed_last_time = 1;
		}

		last_time = current_time;

	}
	Exit();
}

void test_system_idle_instrumented() {
	int i = 0;
	int last_time = 0;
	int current_time = 0;
	int idle_time = 0;
	int total_time = 0;
	int last_output_time = 0;
	unsigned int volatile * timer4_low = (unsigned int volatile *) TIMER4_LOW;
	unsigned int volatile * timer4_high = (unsigned int volatile *) TIMER4_HIGH;

	struct print_buffer pbuff;

	*timer4_high = 0xFFFFFFFF;//TIMER4_ENABLE_MASK;
	for(i = 0; i < 100; i++) {

	}
    i = 0;
	current_time = last_time = *timer4_low;

	while(1) {
		current_time = *timer4_low;

		if(current_time < last_time) {
			last_time = current_time;
			last_output_time = current_time;
			continue;
		}
		total_time += current_time - last_time;

		if(current_time <= last_time + 10) {
			idle_time += current_time - last_time;
		}


		if(current_time > last_output_time + 100000) {
			/*ap_init_buff( &pbuff );
			ap_putstr( &pbuff, "\x1B[?25l");	//hide cursor
			ap_putstr( &pbuff, "\x1B[1;75f");	//move to clock position
			ap_putstr( &pbuff, "\x1B[1;32m");	//set attributes
			ap_printf( &pbuff, "%d %", ( (100 * idle_time) / total_time) );
			ap_putstr( &pbuff, "\x1B[0m");	//clear attributes
			ap_putstr( &pbuff, "\x1B[u");	//return to CL
			ap_putstr( &pbuff, "\x1B[?25h");	//show cursor
			Putbuff( COM2, &pbuff );

			if(i%10 == 0) {
				CommandOutput("total_time: %d idle_time: %d current time %d", total_time, idle_time, current_time);
			}
			*/
			last_output_time = current_time;
			total_time = 0;
			idle_time = 0;
			i++;

			bwprintf(COM2, "%d %d %d\n", last_time, current_time, current_time - last_time);
		}

		last_time = current_time;

	}
}

