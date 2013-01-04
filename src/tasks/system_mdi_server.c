#include <userspace.h>

//marklin digital interface server
void task_system_mdi_server() {
	int recv_tid = 0, i = 0; 
	int train = 0, speed = 0, sw = 0, setting = 0;
	int switch_states[CONFIG_NUM_SWITCHES];
	struct sensor_reply_message sensor_rpl;
	struct number_message rpl;
	struct two_number_message msg;
	int last_sensor_poll_time = 0, current_sensor_poll_time = 0;
	RegisterAs("mdiserv");
	
	//Signal the trains to GO
	Putc(COM1, 96);
	
	//Set sensor module to reset
	Putc(COM1, 192);
	
	for(i = 0; i < CONFIG_NUM_SWITCHES; i++) {
		switch_states[i] = SWITCH_UNKNOWN;
	}
	
	while(1) {
		//get another message, we have reached the end
		Receive(&recv_tid, (char *) &msg, sizeof(msg));
		
		//act on the message type
		switch(msg.message_type) {
			case TRAIN_CTRL_MESSAGE:
				//reply immediately
				train = msg.num1;
				speed = msg.num2;
				rpl.message_type = ACK_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				Putc(COM1, speed);
				Putc(COM1, train);
				break;
			case TRAIN_REVERSE_MESSAGE:
				//reply immediately
				train = msg.num1;
				rpl.message_type = ACK_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				Putc(COM1, 15);
				Putc(COM1, train);
				break;
			case ABORT_MESSAGE:
				Putc(COM1, 97);	//STOP THE TRAIN CONTROLLER
				rpl.message_type = ACK_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				break;				
			case SWITCH_CTRL_MESSAGE:
				//reply immediately
				sw = msg.num1;
				setting = msg.num2;
				rpl.message_type = ACK_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				
				if(setting == SWITCH_STRAIGHT) {
					Putc(COM1, 33);
                                        //DisplaySwitch( sw, 'S');					
				} else if(setting == SWITCH_CURVED) {
					Putc(COM1, 34);
                                        //DisplaySwitch( sw, 'C');					
				}
				Putc(COM1, sw);
				Putc(COM1, 32);
				switch_states[switch_index(sw)] = setting;
				break;			
			case QUERY_ALL_SENSORS_MESSAGE:
				Putc(COM1, 128 + CONFIG_NUM_SENSOR_MODULES);
				int time1, time2, time_diff;
				
				for(i = 0; i < CONFIG_NUM_SENSOR_MODULES * 2; i++) {
					sensor_rpl.sensordata[i] = Getc(COM1);
					if(i == 0) {
						time1 = current_sensor_poll_time = Time();
					}
					//int * mdmctl = (int *)( UART1_BASE + UART_MDMCTL_OFFSET);
					//*mdmctl = *mdmctl | 3;
				}
				
				//time2 = Time();

				//time_diff = time2 - time1;
				//if( time_diff >= 0 ) {
				//	CommandOutput( "\x1B[30;47m[MDISERVER] SENSOR POLL TOOK %d TICKS\x1B[0m", time_diff );
				//}

				if(last_sensor_poll_time > 0) {
					sensor_rpl.time = (current_sensor_poll_time + last_sensor_poll_time) / 2;
				} else {
					sensor_rpl.time = current_sensor_poll_time;
				}

				//reply only after we have something
				sensor_rpl.message_type = SENSOR_REPLY_MESSAGE;
				Reply(recv_tid, (char *) &sensor_rpl, sizeof(sensor_rpl));
				last_sensor_poll_time = current_sensor_poll_time;
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				rpl.num = 0;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				break;
		}
		
	}
}

