#include <userspace.h>

//command functions
void command_tr( struct char_buffer* cbuff );
void command_tdir( struct char_buffer* cbuff );
void command_sw( struct char_buffer* cbuff );
void command_q( struct char_buffer* cbuff );
void command_game( struct char_buffer* cbuff );
void command_wh( struct char_buffer* cbuff );
void command_st( struct char_buffer* cbuff );
void command_swa( struct char_buffer* cbuff );
void command_rv( struct char_buffer* cbuff );
void command_echo( struct char_buffer* cbuff );
void command_delay( struct char_buffer* cbuff );
void command_beep( struct char_buffer* cbuff );
void command_looptest( struct char_buffer* cbuff );
void command_sos( struct char_buffer* cbuff );
void command_zombtest( struct char_buffer* cbuff );
void command_add( struct char_buffer* cbuff, int track );
void command_track( struct char_buffer* cbuff, int* track, struct track_node * track_data );
void command_calibrationout( struct char_buffer* cbuff );
void command_route( struct char_buffer* cbuff, struct track_node * track_data );
void command_resv( struct char_buffer* cbuff, struct track_node * track_data );
void command_free( struct char_buffer* cbuff, struct track_node * track_data );

//helper functions
int validate_char( char* c );
int next_token( struct char_buffer* cbuff, char* output );

//task body


void task_user_cli( ) {
	char c;
	char cbuff_storage[CONFIG_CLI_MAX_CHARACTERS];
	struct char_buffer cbuff;
	cbuffer_init( &cbuff, cbuff_storage, CONFIG_CLI_MAX_CHARACTERS );

	//used for parsing out the first token
	char first_token[CONFIG_CLI_MAX_CHARACTERS];

	//keep track of which track we are
	int track = TRACK_UNINITIALIZED;

	//Copy of track data
	struct track_node track_data[TRACK_MAX];

	//for complex prints
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	//go to the CLI location, and save it
	Printf( COM2, "\x1B[24;20f\x1B[s" );

	while( 1 ) {
		ap_init_buff( &pbuff );
		c = Getc( COM2 );

		//catch special characters
		switch( c ) {
			case CHAR_GRAVE_ACCENT:
				CommandOutput( "[CLI] ABORT!" );
				Abort( );
				break;
			case CHAR_BACKSPACE:
				if( cbuff.state == CBUFF_EMPTY ) {
					break;
				} //do nothing if empty

				//remove the last character
				cbuffer_pop_last( &cbuff );

				//move the cursor back one space and delete the character
				Printf( COM2, "\x1B[u\x1B[D\x1B[K\x1B[s" );

				break;
			case CHAR_PERIOD:
				TabRight( );
				break;
			case CHAR_COMMA:
				TabLeft( );
				break;
			case CHAR_NEWLINE:
			case CHAR_ENTER:
				if( cbuff.state == CBUFF_EMPTY ) {
					break;
				} //do nothing if empty

				//we need to parse, get the first token
				if( next_token( &cbuff, first_token ) == 0 ) {
					break;
				} //we were just spaces

				//now check which command the first token matches
				if( strcmp( first_token, "tr" ) == 0 ) {
					command_tr( &cbuff );
				} else if( strcmp( first_token, "q" ) == 0 ) {
					command_q( &cbuff );
				} else if( strcmp( first_token, "game" ) == 0 ) {
					command_game( &cbuff );
				} else if( strcmp( first_token, "wh" ) == 0 ) {
					command_wh( &cbuff );
				} else if( strcmp( first_token, "st" ) == 0 ) {
					command_st( &cbuff );
				} else if( strcmp( first_token, "sw" ) == 0 ) {
					command_sw( &cbuff );
				} else if( strcmp( first_token, "swa" ) == 0 ) {
					command_swa( &cbuff );
				} else if( strcmp( first_token, "rv" ) == 0 ) {
					command_rv( &cbuff );
				} else if( strcmp( first_token, "add" ) == 0 ) {
					command_add( &cbuff, track );
				} else if( strcmp( first_token, "echo" ) == 0 ) {
					command_echo( &cbuff );
				} else if( strcmp( first_token, "delay" ) == 0 ) {
					command_delay( &cbuff );
				} else if( strcmp( first_token, "beep" ) == 0 ) {
					command_beep( &cbuff );
				} else if( strcmp( first_token, "looptest" ) == 0 ) {
					command_looptest( &cbuff );
				} else if( strcmp( first_token, "zombtest" ) == 0 ) {
					command_zombtest( &cbuff );
				} else if( strcmp( first_token, "track" ) == 0 ) {
					command_track( &cbuff, &track, track_data );
				} else if( strcmp( first_token, "sos" ) == 0 ) {
					command_sos( &cbuff );
				} else if( strcmp( first_token, "calibrationout" ) == 0 ) {
					command_calibrationout( &cbuff );
				} else if( strcmp( first_token, "route" ) == 0 ) {
					command_route( &cbuff, track_data );
				} else if( strcmp( first_token, "tdir" ) == 0 ) {
					command_tdir( &cbuff );
				} else if( strcmp( first_token, "resv" ) == 0 ) {
					command_resv( &cbuff, track_data );
				} else if( strcmp( first_token, "free" ) == 0 ) {
					command_free( &cbuff, track_data );
				} else {
					CommandOutput( "Invalid Command" );
				}


				//empty the command buffer
				cbuffer_empty( &cbuff );
				//clear the command line
				Printf( COM2, "\x1B[24;20f\x1B[K\x1B[s" );

				break;
			default:
				//if it is not a valid character ignore it
				if( !validate_char( &c ) ) {
					break;
				}
				//if we are full, ignore the character
				if( cbuff.state == CBUFF_FULL ) {
					break;
				}

				//we have a valid character buffer and print it
				cbuffer_push_char( &cbuff, c );
				Printf( COM2, "\x1B[u%c\x1B[s", c );
				break;
		}

		//re-initialize the parse
		first_token[0] = '\0';
	}
}



//return the token size, false if there was no token, assumes enough space on output


int next_token( struct char_buffer* cbuff, char* output ) {
	int i = 0;

	//clear the leading space
	while( cbuff->state != CBUFF_EMPTY && cbuffer_peek( cbuff ) == CHAR_SPACE ) {
		//pop the space
		cbuffer_pop( cbuff );
	}

	//we have reached the token
	while( cbuff->state != CBUFF_EMPTY && cbuffer_peek( cbuff ) != CHAR_SPACE ) {
		output[i++] = cbuffer_pop( cbuff );
	}

	//add the null terminator
	output[i] = '\0';
	return i;
}


int validate_char( char* c ) {
	if( *c >= 0x30 && *c <= 0x39 ) { //0-9
		return TRUE;
	}
	if( *c >= 0x61 && *c <= 0x7A ) {
		return TRUE;
	}
	if( *c == 0x20 ) { //space
		return TRUE;
	}
	if( *c >= 0x41 && *c <= 0x5A ) { //uppercase letters
		*c = *c + 32;
		return TRUE;
	}
	return FALSE;
}

//tr train_number train_speed


void command_tr( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg3[CONFIG_CLI_MAX_CHARACTERS];
	int arg1 = 0;
	int arg2 = 0;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) == 0 ||
		!str_is_integer( str_arg2 ) ||
		next_token( cbuff, str_arg3 ) != 0 ) {
		CommandOutput( "[TR]\tInvalid Arguments" );
		return;
	}

	arg1 = str_atoi( str_arg1 );
	arg2 = str_atoi( str_arg2 );

	if( arg2 < 0 || arg2 > 14 ) {
		CommandOutput( "[TR]\tInvalid Train Speed" );
		return;
	}


	if( SetTrainSpeed( arg1, arg2 ) != -1 ) {
		CommandOutput( "[TR]\tSetting Train %d to Speed %d", arg1, arg2 );
	} else {
		CommandOutput( "[TR]\tERROR: Train %d has not been added", arg1 );
	}
}

//tdir train_number train_direction


void command_tdir( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg3[CONFIG_CLI_MAX_CHARACTERS];
	int arg1 = 0;
	int arg2 = 0;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) == 0 ||
		str_arg2[0] == '\0' ||
		str_arg2[1] != '\0' ||
		next_token( cbuff, str_arg3 ) != 0 ) {
		CommandOutput( "[TRDIR]\tInvalid Arguments" );
		return;
	}

	arg1 = str_atoi( str_arg1 );

	if( str_arg2[0] != 'f' && str_arg2[0] != 'b' ) {
		CommandOutput( "[TRDIR]\tInvalid Direction" );
		return;
	}

	if( str_arg2[0] == 'f' ) {
		arg2 = DIRECTION_FORWARD;
	} else {
		arg2 = DIRECTION_BACKWARD;
	}

	//TODO: send direction to train


	if( SetTrainDirection( arg1, arg2 ) != -1 ) {
		switch( arg2 ) {
			case DIRECTION_FORWARD:
				CommandOutput( "[TRDIR]\tSetting Train %d to Direction Forward", arg1 );
				break;
			default:
				CommandOutput( "[TRDIR]\tSetting Train %d to Direction Backward", arg1 );
				break;
		}
	} else {
		CommandOutput( "[TR]\tERROR: Train %d has not been added", arg1 );
	}
}

//sw switch_number switch_direction


void command_sw( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg3[CONFIG_CLI_MAX_CHARACTERS];
	int arg1 = 0;
	char arg2;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) == 0 ||
		str_arg2[0] == '\0' ||
		str_arg2[1] != '\0' ||
		next_token( cbuff, str_arg3 ) != 0 ) {
		CommandOutput( "[SW]\tInvalid Arguments" );
		return;
	}

	arg1 = str_atoi( str_arg1 );

	//check the switch number
	if( switch_index( arg1 ) == -1 ) {
		CommandOutput( "[SW]\tInvalid Switch Number" );
		return;
	}

	//check the switch state
	if( str_arg2[0] != 's' && str_arg2[0] != 'c' ) {
		CommandOutput( "[SW]\tInvalid Switch State" );
		return;
	}

	if( str_arg2[0] == 'c' ) {
		arg2 = (char)SWITCH_CURVED;
		CommandOutput( "[SW]\tSwitching %d To Curved", arg1 );
	} else {
		arg2 = (char)SWITCH_STRAIGHT;
		CommandOutput( "[SW]\tSwitching %d To Straight", arg1 );
	}

	SetSwitch( arg1, arg2 );
}

//q


void command_q( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	if( next_token( cbuff, str_arg1 ) != 0 ) {
		CommandOutput( "[Q]\tInvalid Arguments" );
		return;
	}

	Quit( );
}

void command_game( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	if( next_token( cbuff, str_arg1 ) != 0 ) {
		CommandOutput( "[Q]\tInvalid Arguments" );
		return;
	}

	//hand over the character stream to the game task
	Send( WhoIs("gameKEY"), (char*)0, 0, (char*)0, 0 );
}

//wh


void command_wh( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	if( next_token( cbuff, str_arg1 ) != 0 ) {
		CommandOutput( "[WH]\tInvalid Arguments" );
		return;
	}
	int last_sensor = LastSensor( );
	if( last_sensor > 0 ) {
		CommandOutput( "Sensor %c%d Activated", sensor_letter( last_sensor ), sensor_number( last_sensor ) );
	} else {
		CommandOutput( "No Sensors Have Been Activated" );
	}
}

//st switch_number


void command_st( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	int arg1 = 0;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) != 0 ) {
		CommandOutput( "[ST]\tInvalid Arguments" );
		return;
	}

	arg1 = str_atoi( str_arg1 );

	//check the switch number
	if( switch_index( arg1 ) == -1 ) {
		CommandOutput( "[ST]\tInvalid Switch Number" );
		return;
	}

	int status = SwitchStatus( arg1 );
	//display the state that the switch was last switched into
	if( status == SWITCH_CURVED ) {
		CommandOutput( "[ST]\tSwitch %d is Curved", arg1 );
	} else if( status == SWITCH_STRAIGHT ) {
		CommandOutput( "[ST]\tSwitch %d is Straight", arg1 );
	} else if( status == SWITCH_UNKNOWN ) {
		CommandOutput( "[ST]\tSwitch %d is in an Unknown state", arg1 );
	} else {
		CommandOutput( "[ST]\t%d", status );
	}
}

//swa switch_state


void command_swa( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char arg1;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		str_arg1[0] == '\0' ||
		str_arg1[1] != '\0' ||
		next_token( cbuff, str_arg2 ) != 0 ) {
		CommandOutput( "[SWA]\tInvalid Arguments" );
		return;
	}

	//check the switch state
	if( str_arg1[0] != 's' && str_arg1[0] != 'c' ) {
		CommandOutput( "[SWA]\tInvalid Switch State" );
		return;
	}

	if( str_arg1[0] == 'c' ) {
		arg1 = (char)SWITCH_CURVED;
		CommandOutput( "[SWA]\tSwitching All Switches To Curved" );
	} else {
		arg1 = (char)SWITCH_STRAIGHT;
		CommandOutput( "[SWA]\tSwitching All Switches To Straight" );
	}

	int i = 0;
	for( i = 0; i < 22; i++ ) {
		SetSwitch( switch_index_inverse( i ), arg1 );
	}
}

//rv train_number


void command_rv( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	int arg1 = 0;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) != 0 ) {
		CommandOutput( "[RV]\tInvalid Arguments" );
		return;
	}

	arg1 = str_atoi( str_arg1 );

	if( ReverseTrain( arg1 ) != -1 ) {
		CommandOutput( "[RV]\tReversing Train %d", arg1 );
	} else {
		CommandOutput( "[RV]\tERROR: Train %d has not been added", arg1 );
	}


}

//echo string


void command_echo( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];

	if( next_token( cbuff, str_arg1 ) == 0 ) {
		CommandOutput( "[ECHO]\tInvalid Arguments" );
		return;
	}

	int i = 0;
	while( cbuff->state != CBUFF_EMPTY ) {
		str_arg2[i] = cbuffer_pop( cbuff );
		i++;
	}
	str_arg2[i] = '\0';


	CommandOutput( "%s%s", str_arg1, str_arg2 );
}

//delay clock_ticks


void command_delay( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	int arg1 = 0;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) != 0 ) {
		CommandOutput( "[DELAY]\tInvalid Arguments" );
		return;
	}

	arg1 = str_atoi( str_arg1 );

	//if(arg1 > 1000){ arg1 = 1000; }

	CommandOutput( "[DELAY]\tDelaying For %d", arg1 );
	Delay( arg1 );
}

//beep


void command_beep( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];

	if( next_token( cbuff, str_arg1 ) != 0 ) {
		CommandOutput( "[BEEP]\tInvalid Arguments" );
		return;
	}

	CommandOutput( "[BEEP]\a" );
}

//looptest priority


void command_looptest( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	int arg1 = 0;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) != 0 ) {
		CommandOutput( "[LOOPTEST]\tInvalid Arguments" );
		return;
	}

	arg1 = str_atoi( str_arg1 );

	//check if priority is valid
	if( arg1 < 0 || arg1 >= PRIORITY_NUM_PRIORITIES ) {
		CommandOutput( "[LOOPTEST]\tInvalid Priority" );
		return;
	}

	CommandOutput( "[LOOPTEST]\tStarting Loop Test" );
	Create( arg1, &task_test_loop_time );
}


//sos train_number sensor_name


void command_sos( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg3[CONFIG_CLI_MAX_CHARACTERS];

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) == 0 ||
		next_token( cbuff, str_arg3 ) != 0 ) {
		CommandOutput( "[SOS]\tInvalid Arguments" );
		return;
	}

	int train_num = str_atoi( str_arg1 );

	int sensor_num = 0;
	if( !sensor_str2num( str_arg2, &sensor_num ) ) {
		CommandOutput( "[SOS]\tInvalid Sensor Name" );
		return;
	}

	if( SOSTrain( train_num, sensor_num ) != -1 ) {
		CommandOutput( "[SOS]\tStopping Train [%d] on Trigger of Sensor [%s] aka [%d]", train_num, str_arg2, sensor_num );
	} else {
		CommandOutput( "[SOS]\tERROR: Train %d has not been added", train_num );
	}
}

//zombtest block_size num_blocks


void command_zombtest( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg3[CONFIG_CLI_MAX_CHARACTERS];
	int block_size = 0;
	int num_blocks = 0;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) == 0 ||
		!str_is_integer( str_arg2 ) ||
		next_token( cbuff, str_arg3 ) != 0 ) {
		CommandOutput( "[ZOMB]\tInvalid Arguments" );
		return;
	}

	block_size = str_atoi( str_arg1 );
	num_blocks = str_atoi( str_arg2 );

	CommandOutput( "[ZOMB] STARTING ZOMBIE RECLEMATION TEST [%d blocks of size %d]", num_blocks, block_size );
	struct two_number_message init_msg;
	int zombtesttid = Create( PRIORITY_TEST_ZOMB, &task_test_zombie_reclaim );
	init_msg.num1 = block_size;
	init_msg.num2 = num_blocks;
	Send( zombtesttid, (char *)&init_msg, sizeof (init_msg), (char *)0, 0 );
}

//add train_number


void command_add( struct char_buffer* cbuff, int track ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	int arg1 = 0;

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) != 0 ) {
		CommandOutput( "[ADD]\tInvalid Arguments" );
		return;
	}

	arg1 = str_atoi( str_arg1 );

	if( track == TRACK_UNINITIALIZED ) {
		CommandOutput( "[ADD]\tERROR: No track has been selected" );
		return;
	}

	CommandOutput( "[ADD]\tAdding Train %d", arg1 );
	AddTrain( arg1, track );
}

//swa switch_state


void command_track( struct char_buffer* cbuff, int* track, struct track_node * track_data ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];

	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		str_arg1[0] == '\0' ||
		str_arg1[1] != '\0' ||
		next_token( cbuff, str_arg2 ) != 0 ) {
		CommandOutput( "[TRACK]\tInvalid Arguments" );
		return;
	}

	//check the switch state
	if( str_arg1[0] != 'a' && str_arg1[0] != 'b' ) {
		CommandOutput( "[TRACK]\tInvalid track designation" );
		return;
	}

	if( *track != TRACK_UNINITIALIZED ) {
		CommandOutput( "[TRACK]\tTrack Already Selected" );
		return;
	}

	if( str_arg1[0] == 'a' ) {
		*track = TRACK_A;
		CommandOutput( "[TRACK]\tSelecting Track A" );
		init_tracka( track_data );
	} else {
		*track = TRACK_B;
		CommandOutput( "[TRACK]\tSelecting Track B" );
		init_trackb( track_data );
	}

	//Send a message to the route server, telling them which track should be used
	struct number_message msg, rpl;
	int tid = WhoIs( "routeserv" );
	while( tid < 0 ) {
		tid = WhoIs( "routeserv" );
	}
	msg.message_type = ROUTE_SERVER_INIT_MESSAGE;
	msg.num = *track;
	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (msg) );
}

//calibrationout


void command_calibrationout( struct char_buffer* cbuff ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];

	if( next_token( cbuff, str_arg1 ) != 0 ) {
		CommandOutput( "[CALIB]\tInvalid Arguments" );
		return;
	}

	CommandOutput( "[CALIB] PRINTING CALIBRATION" );

	//tell the trains to print calibration
	PrintTrainCalibration( );
}

//route train_number destination [offset]


void command_route( struct char_buffer* cbuff, struct track_node * track_data ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg3[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg4[CONFIG_CLI_MAX_CHARACTERS];
	int destination = 0, offset = 0;

	int train_num;
	struct train_route_request_message reqmsg;
	struct empty_message rpl;
	int tid = WhoIs( "traindispatch" );


	if(
		next_token( cbuff, str_arg1 ) == 0 ||
		!str_is_integer( str_arg1 ) ||
		next_token( cbuff, str_arg2 ) == 0) {
		CommandOutput( "[ROUTE]\tInvalid Arguments" );
		return;
	}
	if( next_token( cbuff, str_arg3 ) != 0 ) {
		offset = str_atoi( str_arg3 );
	}
	if(	next_token( cbuff, str_arg4 ) != 0 ) {
		CommandOutput( "[ROUTE]\tInvalid Arguments" );
		return;
	}

	train_num = str_atoi( str_arg1 );
	if( !track_str2num( track_data, str_arg2, &destination ) ) {
		CommandOutput( "[ROUTE]\tInvalid Landmark Name" );
		return;
	}


	reqmsg.message_type = TRAIN_ROUTE_REQUEST_MESSAGE;
	reqmsg.train = train_num;
	reqmsg.pos1.node = -1;
	reqmsg.pos1.offset = 0;
	reqmsg.pos2.node = destination;
	reqmsg.pos2.offset = offset;

	Send( tid, (char *)&reqmsg, sizeof (reqmsg), (char *)&rpl, sizeof (rpl) );

	CommandOutput( "[ROUTE] Routing Train %d to %s + %d mm", train_num, track_data[destination].name, offset );

}

void command_resv( struct char_buffer* cbuff, struct track_node * track_data ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg3[CONFIG_CLI_MAX_CHARACTERS];

	int node = 0, edge = 0;

	struct two_number_message msg;
	struct empty_message rpl;
	int tid = WhoIs( "routeserv" );

	if( next_token( cbuff, str_arg1 ) == 0) {
		CommandOutput( "[RESV]\tInvalid Arguments" );
		return;
	}

	if( !track_str2num( track_data, str_arg1, &node ) ) {
		CommandOutput( "[RESV]\tInvalid Landmark Name" );
		return;
	}

	if( next_token( cbuff, str_arg2 ) != 0 && !(str_arg2[0] == '\0' || str_arg2[1] != '\0')) {
		//check the switch state

		if( str_arg2[0] == 'c' ) {
			edge = DIR_CURVED;
		} else if( str_arg2[0] == 's' ){
			edge = DIR_STRAIGHT;
		} else {
			CommandOutput( "[RESV]\tInvalid Switch State" );
			return;
		}
		if(	next_token( cbuff, str_arg3 ) != 0 ) {
			CommandOutput( "[RESV]\tInvalid Arguments" );
			return;
		}
	}

	msg.message_type = TRAIN_ROUTE_DEBUG_RESERVE_MESSAGE;
	msg.num1 = node;
	msg.num2 = edge;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	CommandOutput( "[RESV] Reserving Track Section %s(%d)", track_data[node].name, edge );
}

void command_free( struct char_buffer* cbuff, struct track_node * track_data ) {
	char str_arg1[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg2[CONFIG_CLI_MAX_CHARACTERS];
	char str_arg3[CONFIG_CLI_MAX_CHARACTERS];

	int node = 0, edge = 0;

	struct two_number_message msg;
	struct empty_message rpl;
	int tid = WhoIs( "routeserv" );

	if( next_token( cbuff, str_arg1 ) == 0) {
		CommandOutput( "[FREE]\tInvalid Arguments" );
		return;
	}

	if( !track_str2num( track_data, str_arg1, &node ) ) {
		CommandOutput( "[FREE]\tInvalid Landmark Name" );
		return;
	}

	if( next_token( cbuff, str_arg2 ) != 0 && !(str_arg2[0] == '\0' || str_arg2[1] != '\0')) {
		//check the switch state

		if( str_arg2[0] == 'c' ) {
			edge = DIR_CURVED;
		} else if( str_arg2[0] == 's' ){
			edge = DIR_STRAIGHT;
		} else {
			CommandOutput( "[FREE]\tInvalid Switch State" );
			return;
		}
		if(	next_token( cbuff, str_arg3 ) != 0 ) {
			CommandOutput( "[FREE]\tInvalid Arguments" );
			return;
		}
	}

	msg.message_type = TRAIN_ROUTE_DEBUG_FREE_MESSAGE;
	msg.num1 = node;
	msg.num2 = edge;

	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof (rpl) );

	CommandOutput( "[FREE] Unreserving Track Section %s(%d)", track_data[node].name, edge );
}
