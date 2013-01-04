#include <userspace.h>


void task_system_sensor_poller() {

	struct sensor_reply_message msg;
	struct empty_message rpl;
	int sensor_server_tid = MyParentTid();
	int mditid = WhoIs( "mdiserv" );
	struct two_number_message two_number_msg;
	while(mditid < 0) {
		Delay(2);
		mditid = WhoIs( "mdiserv" );
	}
	while(1) {
		two_number_msg.message_type = QUERY_ALL_SENSORS_MESSAGE;
		Send( mditid, (char *)&two_number_msg, sizeof (two_number_msg), (char *)&msg, sizeof (msg) );
		Send(sensor_server_tid, (char *)&msg, sizeof(msg), (char *) &rpl, sizeof(rpl) );
	}
}


void task_system_sensor_server() {		
	char sensordata[CONFIG_SENSOR_RESULT_LENGTH];
	char lastsensordata[CONFIG_SENSOR_RESULT_LENGTH];
	int sensor_list[CONFIG_SENSOR_LIST_LENGTH];
	int sensor_change = 0;
	
	int tid, recv_tid;
	struct sensor_reply_message msg;
	struct number_message rpl;
	struct sensor_list_message senlistrpl;
	
	int i = 0, j = 0, k = 0, l = 0, m = 0;
	char modulechar = 0;
	char data = 0, lastdata = 0;
	
	RegisterAs("sensorserv");
	
	for(i = 0; i < 7; i++) {
		sensor_list[i] = -1;
	}
	for(i = 0; i < CONFIG_SENSOR_RESULT_LENGTH; i++) {
		sensordata[i] = -1;
		lastsensordata[i] = -1; 
	}
		
	Create(PRIORITY_SENSOR_POLLER, &task_system_sensor_poller);

	int sensor_register_buff_mem[10];	
	struct int_buffer sensor_register_buff;
	intbuffer_init(&sensor_register_buff, sensor_register_buff_mem, 10 );	
	
	while(1) {
		
		Receive(&recv_tid, (char *) &msg, sizeof(msg));
		//act on the message type
		switch(msg.message_type) {
			case SENSOR_REPLY_MESSAGE:
				Reply(recv_tid, 0, 0);
				for(i = 0; i < CONFIG_SENSOR_RESULT_LENGTH; i++) {
					sensordata[i] = msg.sensordata[i];
				}				
				modulechar = 'A';
				for(i = 0; i < CONFIG_NUM_SENSOR_MODULES; i++) {
					for(j = 0; j < 2; j++) {
						m = 128;
						data = sensordata[i * 2 + j];
						lastdata = lastsensordata[i * 2 + j];
						for(k = 0; k < 8; k++) {
							if((data & m) && !(lastdata & m)) {
								for(l = CONFIG_SENSOR_LIST_LENGTH - 2; l >= 0 ; l--) {
									sensor_list[l+1] = sensor_list[l];
								}
								sensor_list[0] = (k + (8 * j)) + ((modulechar - 'A') * 16);
								sensor_change++;
							}
							m >>= 1;
						}
					}
					modulechar++;			
				}
				for(i = 0; i < CONFIG_SENSOR_RESULT_LENGTH; i++) {
					lastsensordata[i] = sensordata[i];
				}
				
				if(sensor_change > 0) {
					DisplaySensorList(sensor_list);
					tid = intbuffer_pop( &sensor_register_buff );
					senlistrpl.message_type = SENSOR_SERVER_SENSOR_LIST_MESSAGE;
					for(i = 0; i < CONFIG_SENSOR_LIST_LENGTH; i++) {
						if(i < sensor_change) {
							senlistrpl.sensor_list[i] = sensor_list[i];
						} else {
							senlistrpl.sensor_list[i] = -1;
						}
					}
					senlistrpl.time = msg.time;
					Reply(tid, (char *) &senlistrpl, sizeof(senlistrpl));
					sensor_change = 0;
				}
				break;
			case SENSOR_SERVER_LAST_SENSOR_QUERY_MESSAGE:
				rpl.message_type = SENSOR_SERVER_LAST_SENSOR_REPLY_MESSAGE;				
				rpl.num = sensor_list[0];
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				break;
			case COURIER_MESSAGE:
			case SENSOR_REGISTER_MESSAGE:
				intbuffer_push( &sensor_register_buff, recv_tid);				
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

