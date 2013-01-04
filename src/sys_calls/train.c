#include <sys_calls/sys_calls.h>

#include <userspace.h>


void UpdatePositionDisplay( int train_number, int speed, char* pos_node, int pos_offset, int error, int max_error, char* dest_node, int dest_offset, char* state ) {
	int tid = -1;
	tid = WhoIs( "trainsTAB" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return;
	}

	//create the message
	struct train_position_output_message msg;

	msg.message_type = TRAIN_POSITION_OUTPUT_MESSAGE;
	msg.train_nums = train_number;
	msg.train_speeds = speed;
	safestrcpy( msg.train_pos_node, pos_node, 6 );
	msg.train_pos_off = pos_offset;
	msg.train_err = error;
	msg.train_max_err = max_error;
	safestrcpy( msg.train_dest_node, dest_node, 6 );
	msg.train_dest_off = dest_offset;
	safestrcpy( msg.train_state, state, 6 );

	//send the message via courier
	CourierSend( tid, (char *)&msg, sizeof (msg) );
}

void TrainLocationTabRegister( int train_number, struct train_data* data_pointer ) {
	int tid = -1;
	tid = WhoIs( "TRlocTAB" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return;
	}

	//create the message
	struct two_number_message msg;

	msg.message_type = TRAIN_OUTPUT_REGISTER_MESSAGE;
	msg.num1 = train_number;
	msg.num2 = (int) data_pointer;

	//send the message via courier
	CourierSend( tid, (char *)&msg, sizeof (msg) );
}

void TrainGameAIRegister(int train_number, struct train_data* data_pointer){
	int tid = -1;
	tid = WhoIs( "gameAI" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return;
	}

	//create the message
	struct two_number_message msg;

	msg.message_type = GAME_AI_REGISTER_TRAIN;
	msg.num1 = train_number;
	msg.num2 = (int) data_pointer;

	//send the message via courier
	CourierSend( tid, (char *)&msg, sizeof (msg) );
}


void Abort( ) {
	struct two_number_message msg;
	int tid = -1;

	tid = WhoIs( "mdiserv" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return;
	}

	msg.message_type = ABORT_MESSAGE;

	CourierSend( tid, (char *)&msg, sizeof (msg));
}


void PrintTrainCalibration( ) {
	struct number_message rpl;
	struct two_number_message msg;
	int tid = -1;

	tid = WhoIs( "traindispatch" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return;
	}

	msg.message_type = TRAIN_PRINT_CALIBRATION_MESSAGE;
	msg.num1 = 0;
	msg.num2 = 0;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );
}


int SetTrainSpeed( int train, int speed ) {
	struct number_message rpl;
	struct two_number_message msg;
	int tid = -1, status = 0;

	tid = WhoIs( "traindispatch" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = TRAIN_CTRL_MESSAGE;
	msg.num1 = train;
	msg.num2 = speed;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || ((rpl.message_type & MESSAGE_SEMANTIC_MASK) != SEMANTIC_ACK) ) {
		return -1;
	}

	return 0;
}

int SetTrainDirection( int train, int direction ) {
	struct number_message rpl;
	struct two_number_message msg;
	int tid = -1, status = 0;

	tid = WhoIs( "traindispatch" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = TRAIN_DIRECTION_MESSAGE;
	msg.num1 = train;
	msg.num2 = direction;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || ((rpl.message_type & MESSAGE_SEMANTIC_MASK) != SEMANTIC_ACK) ) {
		return -1;
	}

	return 0;
}


int ReverseTrain( int train ) {
	struct number_message rpl;
	struct two_number_message msg;
	int tid = -1, status = 0;

	tid = WhoIs( "traindispatch" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = TRAIN_REVERSE_MESSAGE;
	msg.num1 = train;
	msg.num2 = 0;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || ((rpl.message_type & MESSAGE_SEMANTIC_MASK) != SEMANTIC_ACK) ) {
		return -1;
	}

	return 0;
}


int AddTrain( int train, int track ) {
	struct number_message rpl;
	struct two_number_message msg;
	int tid = -1, status = 0;

	tid = WhoIs( "traindispatch" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = TRAIN_ADD_MESSAGE;
	msg.num1 = train;
	msg.num2 = track;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || ((rpl.message_type & MESSAGE_SEMANTIC_MASK) != SEMANTIC_ACK) ) {
		return -1;
	}

	return 0;
}


int SOSTrain( int train, int sensor ) {
	struct number_message rpl;
	struct two_number_message msg;
	int tid = -1, status = 0;

	tid = WhoIs( "traindispatch" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = TRAIN_SOS_MESSAGE;
	msg.num1 = train;
	msg.num2 = sensor;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || ((rpl.message_type & MESSAGE_SEMANTIC_MASK) != SEMANTIC_ACK) ) {
		return -1;
	}

	return 0;
}


int SetSwitch( int sw, int setting ) {
	struct number_message rpl;
	struct two_number_message msg;
	int tid = -1, status = 0;

	if( setting != SWITCH_STRAIGHT && setting != SWITCH_CURVED ) {
		return -2;
	}

	tid = WhoIs( "switchserv" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = SWITCH_CTRL_MESSAGE;
	msg.num1 = sw;
	msg.num2 = setting;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || ((rpl.message_type & MESSAGE_SEMANTIC_MASK) != SEMANTIC_ACK) ) {
		return -1;
	}

	return 0;
}


int SwitchStatus( int sw ) {
	struct number_message rpl;
	struct two_number_message msg;
	int tid = -1, status = 0;

	tid = WhoIs( "switchserv" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = SWITCH_STATUS_MESSAGE;
	msg.num1 = sw;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || ((rpl.message_type & MESSAGE_SEMANTIC_MASK) != SEMANTIC_ACK) ) {
		return -1;
	}

	return rpl.num;
}


int LastSensor( ) {
	struct number_message rpl;
	struct number_message msg;
	int tid = -1, status = 0;

	tid = WhoIs( "sensorserv" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = SENSOR_SERVER_LAST_SENSOR_QUERY_MESSAGE;
	//msg.num = sw;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || (rpl.message_type != SENSOR_SERVER_LAST_SENSOR_REPLY_MESSAGE) ) {
		return -1;
	}

	return rpl.num;
}


int QueryAllSensors( char * sensorbuf ) {
	struct sensor_reply_message rpl;
	struct two_number_message msg;
	int tid = -1, status = 0, i = 0;

	tid = WhoIs( "mdiserv" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return -1;
	}

	msg.message_type = QUERY_ALL_SENSORS_MESSAGE;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	if( status < 0 || (rpl.message_type == SENSOR_REPLY_MESSAGE) ) {
		return -1;
	}

	for( i = 0; i < CONFIG_SENSOR_RESULT_LENGTH; i++ ) {
		sensorbuf[i] = rpl.sensordata[i];
	}

	return 0;
}

