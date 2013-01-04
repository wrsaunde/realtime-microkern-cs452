#include <userspace.h>


void task_system_sensor_interpreter() {
	int sensor_server_tid = -1;
	int i = 0, j = 0, k = 0;
	char buffer[512];
	
	int sensor_list[CONFIG_SENSOR_LIST_LENGTH];
	int sensor_attribution[CONFIG_SENSOR_LIST_LENGTH];
	int sensor_time[CONFIG_SENSOR_LIST_LENGTH];

	int current_time = 0;
	struct number_message rpl;
	struct sensor_watch_reply_message watchrpl;
	struct empty_message * emptymsg;
	struct sensor_list_message * sensormsg;
	struct sensor_watch_message myrequests[CONFIG_SENSOR_INTERPRETER_MAX_REQUESTS];
	
	RegisterAs("sensorinterp");	

	while(sensor_server_tid < 0) {
		sensor_server_tid = WhoIs("sensorserv");
		Delay(2);
	}	
	
	//initialize the couriers
	for(i = 0; i < CONFIG_SENSOR_INTERPRETER_NUM_COURIERS; i++){
		Courier(sensor_server_tid, MyTid());		
	}	
	
	for(i = 0; i < CONFIG_SENSOR_INTERPRETER_MAX_REQUESTS; i++) {
		myrequests[i].type = SENSOR_WATCH_TYPE_EXPIRED;
	}
	
	
	while(1) {
		int recv_size = 0;
		int recv_tid = -1;
		//get the next request
		recv_size = Receive(&recv_tid, buffer, 512);

		emptymsg = (struct empty_message *) buffer;
		
		current_time = Time();

		//Check for timeouts
		for(i = 0; i < CONFIG_SENSOR_INTERPRETER_MAX_REQUESTS; i++) {
			if(myrequests[i].type == SENSOR_WATCH_TYPE_SENSOR_TIMEOUT) {
				if(current_time >= myrequests[i].waketime) {
					//CommandOutput("[Sensor Interpreter] Responding to timeout");
					sensor_attribution[j] = myrequests[i].tid;

					watchrpl.message_type = SENSOR_WATCH_REPLY_MESSAGE;
					watchrpl.sensor = -1;
					watchrpl.id = myrequests[i].id;
					watchrpl.time = current_time;

					CourierSend(myrequests[i].tid, (char *) &watchrpl, sizeof(watchrpl));

					myrequests[i].type = SENSOR_WATCH_TYPE_EXPIRED;
				}	
			}
		}

		//act on the message type
		switch(*((int *) buffer)) {
			case SENSOR_SERVER_SENSOR_LIST_MESSAGE:
				Reply(recv_tid, 0, 0);
				
				sensormsg = (struct sensor_list_message *) buffer;
								
				for(i = 0; i < CONFIG_SENSOR_LIST_LENGTH; i++) {
					/*if(sensormsg->sensor_list[i] >= 0) {
						CommandOutput("Got sensor %d", sensormsg->sensor_list[i]);
					}*/
					sensor_list[i] = sensormsg->sensor_list[i];
					sensor_attribution[i] = 0;
					sensor_time[i] = sensormsg->time + sensor_list[i] / 16;
				}
				
				//interpet message
				for(i = 0; i < CONFIG_SENSOR_INTERPRETER_MAX_REQUESTS; i++) {
					if(myrequests[i].type == SENSOR_WATCH_TYPE_SENSOR || myrequests[i].type == SENSOR_WATCH_TYPE_SENSOR_TIMEOUT) {
						for(k = 0; k < myrequests[i].num_sensors; k++) {
							for( j = 0; j < CONFIG_SENSOR_LIST_LENGTH; j++) {
								if (sensor_list[j] < 0) {
									break;
								}
								if(sensor_list[j] == myrequests[i].sensors[k]) {
									sensor_attribution[j] = myrequests[i].tid;
									
									watchrpl.message_type = SENSOR_WATCH_REPLY_MESSAGE;
									watchrpl.sensor = sensor_list[j];
									watchrpl.id = myrequests[i].id;
									watchrpl.time = sensor_time[j];
									CourierSend(myrequests[i].tid, (char *) &watchrpl, sizeof(watchrpl));

									myrequests[i].type = SENSOR_WATCH_TYPE_EXPIRED;
									
									k = myrequests[i].num_sensors;
									break;
								}	
							}
						}
					}
				}
				for(i = 0; i < CONFIG_SENSOR_INTERPRETER_MAX_REQUESTS; i++) {
					if(myrequests[i].type == SENSOR_WATCH_TYPE_UNATTRIBUTED_SENSOR) {
						for( j = 0; j < CONFIG_SENSOR_LIST_LENGTH; j++) {
							if (sensor_list[j] < 0) {
								break;
							} else {
								if(sensor_attribution[j] == 0) {
									watchrpl.message_type = SENSOR_WATCH_REPLY_MESSAGE;
									watchrpl.sensor = sensor_list[j];
									watchrpl.id = myrequests[i].id;
									watchrpl.time = sensor_time[j];
									CourierSend(myrequests[i].tid, (char *) &watchrpl, sizeof(watchrpl));

									myrequests[i].type = SENSOR_WATCH_TYPE_EXPIRED;
									j = CONFIG_SENSOR_LIST_LENGTH;
								}
							}
						}
					}
				}
				break;
			case SENSOR_WATCH_MESSAGE:
				for(i = 0; i < CONFIG_SENSOR_INTERPRETER_MAX_REQUESTS; i++) {
					struct sensor_watch_message* msg = (struct sensor_watch_message*) buffer;
					if(myrequests[i].type == SENSOR_WATCH_TYPE_EXPIRED || myrequests[i].tid == msg->tid) {
						mem_copy(buffer, (char *) (myrequests + i), sizeof(struct sensor_watch_message));
						if(myrequests[i].type == SENSOR_WATCH_TYPE_SENSOR_TIMEOUT) {
							WorkerDelay(myrequests[i].timeout);
							myrequests[i].waketime = myrequests[i].timeout + Time();
						}
						break;
					}
				}
				rpl.message_type = ACK_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));				
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				break;
		}
	}
}

