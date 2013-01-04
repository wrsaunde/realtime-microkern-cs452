#include <userspace.h>

//helper functions
void initialize_tids(int * train_tids);
int get_train_tid(int* train_tids, int train_num);
//end of helpers

void task_user_train_command_dispatcher() {
	
	//array of train tids
	int train_tids[CONFIG_NUM_TRAINS + 1];
	initialize_tids(train_tids);

	const int buffer_size = 100;
	char buffer[buffer_size];
	assert(buffer_size > sizeof(struct train_route_request_message), "buffer_size > sizeof(struct train_route_request_message)");
	
	//msgs
	struct two_number_message msg;
	struct number_message rpl;
	int recv_tid;
	
	//register as the dispatcher
	RegisterAs("traindispatch");
	
	while(1) {
		//get another message, we have reached the end
		Receive(&recv_tid, buffer, buffer_size);

		mem_copy(buffer, (char *) &msg, sizeof(msg));
		
		int train_tid = -1;
		
		//check message type and train tid
		switch(msg.message_type) {
			case TRAIN_ADD_MESSAGE:
				train_tid = get_train_tid(train_tids, train_alias( msg.num1 ));
				if(train_tid == -1){
					//create the new train
					train_tid = Train(msg.num1, msg.num2);
					train_tids[train_alias( msg.num1 )] = train_tid;
				}
			case TRAIN_REVERSE_MESSAGE:
			case TRAIN_DIRECTION_MESSAGE:
			case TRAIN_SOS_MESSAGE:
			case TRAIN_CTRL_MESSAGE:
			case TRAIN_ROUTE_DESTINATION_MESSAGE:
			case TRAIN_ROUTE_REQUEST_MESSAGE:
				train_tid = get_train_tid(train_tids, train_alias(msg.num1));
				if(train_tid == -1){
					//the train hasn't been created, reply and say so
					rpl.message_type = TRAIN_DOESNT_EXIST_MESSAGE;
					rpl.num = 0;
					Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				}else{
					//tell them train exists, command is going to execute
					rpl.message_type = ACK_MESSAGE;
					rpl.num = 0;
					Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				}
				break;
			case TRAIN_GUESS_POSITION_MESSAGE:
				train_tid = get_train_tid(train_tids, train_alias(msg.num1));
				if(train_tid == -1){
					//the train hasn't been created, reply and say so
					rpl.message_type = TRAIN_DOESNT_EXIST_MESSAGE;
					rpl.num = 0;
					Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				}
				break;				
			case TRAIN_PRINT_CALIBRATION_MESSAGE:
				//release the command lind
				rpl.message_type = ACK_MESSAGE;
				rpl.num = 0;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				
				//now clear the screen
				HijackCOM2();
				Printf(COM2, "\x1B[2J\x1B[r\x1B[H");
				Printf(COM2, "\t\t[PRINTING TRAIN CALIBRATION]\n");
				
				//tell each train to print their calibration
				int i = 0;
				for(i = 0; i <= CONFIG_NUM_TRAINS; i++){
					if(train_tids[i] != -1){ 
						//tell the train to print its calibration
						Send(train_tids[i], (char *) &msg, sizeof(msg), (char *) 0, 0);
						//take back COM2
						HijackCOM2();
					}
				}
				
				//shut'er down
				Printf(COM2, "\t\t[FINISHED PRINTING TRAIN CALIBRATION]\n");
				Delay(25);
				Quit();
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				rpl.num = 0;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				break;
		}
		
		if( train_tid == -1 ){ continue; }	//we didn't end up with a valid TID
		
		
		//act on the message type
		switch(msg.message_type) {
			case TRAIN_ADD_MESSAGE:
				//do nothing, we already added it above
				break;
			case TRAIN_CTRL_MESSAGE:
			case TRAIN_DIRECTION_MESSAGE:
			case TRAIN_SOS_MESSAGE:
			case TRAIN_REVERSE_MESSAGE:
			case TRAIN_GUESS_POSITION_MESSAGE:
			case TRAIN_ROUTE_DESTINATION_MESSAGE:
				//forward the message to the train
				Send(train_tid, buffer, sizeof(msg), (char *) &rpl, sizeof(rpl));
				break;		
			case TRAIN_ROUTE_REQUEST_MESSAGE:
				//forward the route request message to the train
				Send(train_tid, buffer, sizeof(struct train_route_request_message), (char *) &rpl, sizeof(rpl));
				break;
		}	
	}
}

int get_train_tid(int* train_tids, int train_num){
	if( train_num < 1 || train_num > CONFIG_NUM_TRAINS ){ return -1; }
	return train_tids[train_num];
}

void initialize_tids(int * train_tids){
	int i = 0;
	for(i = 0; i <= CONFIG_NUM_TRAINS; i++){
		train_tids[i] = -1;
	}
}

