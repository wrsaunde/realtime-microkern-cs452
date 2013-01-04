#include <userspace.h>

//needed functions
void dump_calibration( struct train_data* DATA );

void check_distances( struct train_data* DATA );

int optimal_speed( struct train_data* DATA, int safe_dist, int destination_dist );

void begin_route( struct train_data* DATA );

//


void train_initialize( struct train_data* DATA ) {
	//Get initialization message from creator
	int tid;
	struct two_number_message init_msg;
	Receive( &tid, (char *)&init_msg, sizeof (init_msg) );
	Reply( tid, (char *)0, 0 );
	int tnum = init_msg.num1;
	DATA->train_number = train_alias( tnum );
	DATA->track_number = init_msg.num2;

	//initialize calibration data
	if( DATA->track_number == TRACK_A ) {
		init_tracka( DATA->track );
	} else if( DATA->track_number == TRACK_B ) {
		init_trackb( DATA->track );
	}

	//Initialize train
	init_train_calibration( &(DATA->calibration), tnum );
	DATA->state.direction = DIRECTION_UNKNOWN;

	//initialize the important tids
	DATA->tid_me = MyTid( );
	DATA->tid_mdi = WhoIs( "mdiserv" );
	DATA->tid_sensor_interp = WhoIs( "sensorinterp" );
	DATA->tid_route_serv = WhoIs( "routeserv" );

	//initialize destination information
	DATA->dest.node = -1;
	DATA->dest.offset = 0;

	//initialize the state
	DATA->state.current = STATE_STOPPED;
	DATA->state.lost = TRUE;
	DATA->state.state_change_start_time = 0;
	DATA->state.state_change_finish_time = INT_MAX;
	DATA->state.changed_speed = TRUE;


	//initialize position data
	DATA->state.state_change_pos.node = -1;
	DATA->state.state_change_pos.offset = 0;
	DATA->state.last_sensor_pos.node = -1;
	DATA->state.last_sensor_pos.offset = 0;
	DATA->state.current_pos_guess.node = -1;
	DATA->state.current_pos_guess.offset = 0;

	//initialize speed data
	DATA->state.speed_last = 0;
	DATA->state.speed_target = 0;
	DATA->state.velocity_last.d = 0;
	DATA->state.velocity_last.t = 1;
	DATA->state.velocity_target.d = 0;
	DATA->state.velocity_target.t = 1;

	//initialize the error
	DATA->error_last = 0;
	DATA->error_max = 0;

	DATA->last_pos_notify_time = 0;
	DATA->last_switch_time = 0;
	DATA->last_lost_check_time = 0;
	DATA->stop_worker_running = FALSE;

	DATA->last_res_update_time = 0;
	DATA->waiting_for_res = FALSE;

	//initialize the train structure
	init_route( &DATA->route );
	init_route( &DATA->route_buffer );

	DATA->on_secondary_route = FALSE;

	//stop the train from moving
	mdi_set_speed( DATA, 0 );

	//start a periodic delay worker to keep waking me
	WorkerPeriodicDelay( 15 );
}


//ACTUAL TASK


void task_user_train( ) {
	//msgs
	struct two_number_message msg, twonumrpl;
	struct empty_message rpl;

	char buffer[CONFIG_TRAIN_MESSAGE_BUFFER_LENGTH];

	//train data store
	struct train_data data;
	struct train_data* DATA = &data;

	//initialize the train
	train_initialize( DATA );

	//watch for sensors to find position
	create_unatrib_sensor_watch( DATA );

	TrainLocationTabRegister( DATA->train_number, DATA );
	TrainGameAIRegister(DATA->train_number, DATA);

	//loop doing train things
	while( 1 ) {
		//receive the next message
		int recv_tid;
		//CommandOutput("Leaving Train Task %d", DATA->tid_me);
		//Delay(2);
		Receive( &recv_tid, buffer, CONFIG_TRAIN_MESSAGE_BUFFER_LENGTH );

		mem_copy( buffer, (char *)&msg, sizeof (msg) );
		//CommandOutput("Starting Train Task %d, message_type: %d", DATA->tid_me, msg.message_type);
		//Delay(2);
		//get the current time, used heavily throughout the processing
		int tinit = DATA->current_time = Time( );

		//refresh our state
		refresh_state( DATA );
		//predict the current location
		DATA->state.current_pos_guess = guess_position( DATA );
		//now deal with the message

		if(DATA->state.current_pos_guess.node < 0 && !DATA->state.lost && DATA->state.current != STATE_STOPPING_REVERSE && DATA->state.current != STATE_STOPPED_REVERSE) {
			CommandOutput("\x1B[43;30m[TR%d] UPDATE LOST 3\x1B[0m", DATA->train_number);
			update_state_lost(DATA);
		}

		switch( msg.message_type ) {
				/* TRAIN COMMANDS FROM THE DISPATCHER */
			case TRAIN_REVERSE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				if( DATA->state.current == STATE_STOPPING_REVERSE || DATA->state.current == STATE_STOPPED_REVERSE ) {
					CommandOutput( "[TRAIN][%d] Ignoring Reverse, already reversing", DATA->train_number );
				} else {
					reverse_train( DATA );
				}
				break;
			case TRAIN_CTRL_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				if( msg.num1 == DATA->train_number ) {
					set_train_speed( DATA, msg.num2 );
				}
				break;
			case TRAIN_DIRECTION_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				if( msg.num1 == DATA->train_number ) {
					DATA->state.direction = msg.num2;
				}
				break;
			case TRAIN_SOS_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				/*if(msg.num1 == DATA->train_number){
					stop_me_on_sensor(DATA, msg.num2);
				}*/
				break;
			case TRAIN_PRINT_CALIBRATION_MESSAGE:
				HijackCOM2( );
				dump_calibration( DATA ); //print the calibration,
				Reply( recv_tid, (char *)0, 0 ); //release the dispatcher
				Delay( 10000 ); //block so we don't mess things up
				break;
				/* END OF DISPATCHER TRAIN COMMANDS */

				/* WAKEUP MESSAGE */
			case DELAY_WORKER_MESSAGE:
				//send the delay worker away
				Reply( recv_tid, (char *)0, 0 );
				if( msg.num1 == DELAY_STOP_WAKEUP ) {
					//we have awoken from the stop worker
					DATA->stop_worker_running = FALSE;
				}
				break;
				/* END OF WAKEUP */
			case TRAIN_GUESS_POSITION_MESSAGE:
				twonumrpl.message_type = TRAIN_GUESS_POSITION_RESPONSE_MESSAGE;
				twonumrpl.num1 = DATA->state.current_pos_guess.node;
				twonumrpl.num2 = DATA->state.current_pos_guess.offset;
				if( msg.num2 >= 0 ) {
					Reply( msg.num2, (char *)&twonumrpl, sizeof (twonumrpl) );
				}
				twonumrpl.message_type = TRAIN_GUESS_POSITION_RESPONSE_MESSAGE;
				twonumrpl.num1 = DATA->state.current_pos_guess.node;
				twonumrpl.num2 = DATA->state.current_pos_guess.offset;
				Reply( recv_tid, (char *)&twonumrpl, sizeof (twonumrpl) );
				break;
			case SENSOR_WATCH_REPLY_MESSAGE:

				Reply( recv_tid, (char *)0, 0 );

				struct sensor_watch_reply_message *wrmsg = (struct sensor_watch_reply_message *)buffer;

				int time_diff = DATA->current_time - wrmsg->time;
				if( time_diff > 7 ) {
					CommandOutput( "\x1B[30;47m[TR%d] SENSOR HANDLE TO TRAIN TASK TOOK %d TICKS\x1B[0m", DATA->train_number, time_diff );
				}

				//CommandOutput("[Train %d] Got a sensor watch back %d %d", DATA->number, msg.num1, msg.num2);
				if( wrmsg->id == SOS_SENSOR_WATCH ) {
					set_train_speed( DATA, 0 );
				} else if( wrmsg->id == UNATTRIBUTED_SENSOR_WATCH || wrmsg->id == NORMAL_SENSOR_WATCH ) {
					update_state_from_sensor( DATA, wrmsg->sensor, wrmsg->time );
				}

				break;
			case TRAIN_ROUTE_REQUEST_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				if( !DATA->state.lost ) {
					struct train_route_request_message *rrmsg = (struct train_route_request_message *)buffer;
					DATA->dest.node = rrmsg->pos2.node;
					DATA->dest.offset = rrmsg->pos2.offset;

					rrmsg->pos1.node = DATA->state.current_pos_guess.node;
					rrmsg->pos1.offset = DATA->state.current_pos_guess.offset;
					rrmsg->train_tid = DATA->tid_me;
					rrmsg->train = DATA->train_number;
					rrmsg->requested_route_length = CONFIG_TRAIN_REQUESTED_ROUTE_LENGTH;

					//Pointer for ungodly pointer copy  NEVER DO THIS AGAIN
					//(If you do decide to do this again, at least copy this warning message)
					rrmsg->route = &DATA->route_buffer;

					//CommandOutput( "[TRAIN %d] Routing to node %s ", DATA->train_number, DATA->track[DATA->dest.node].name, DATA->dest.node );
					CourierSend( DATA->tid_route_serv, (char*)rrmsg, sizeof (struct train_route_request_message) );
				} else {
					CommandOutput( "[TRAIN][%d] Ignoring route, I am lost", DATA->train_number );
				}

				break;
			case TRAIN_ROUTE_MESSAGE:

				//We've been given a new route! Copy it into the data struct;
				//CommandOutput("Copying Route");
				copy_route( &(DATA->route_buffer), &(DATA->route) );
				DATA->dest = DATA->route.destination;
				//mem_copy( (char *)&(DATA->route_buffer), (char *)&(DATA->route), sizeof (struct train_route) );
				CommandOutput("Copied the Route");
				Reply( recv_tid, (char *)0, 0 );

				//TODO: Any processing/state change we need to do upon getting a new route.
				//notify_position( DATA );

				//Delay(10);
				//display_route( DATA->track, &(DATA->route) );
				//Delay(10);
				//CommandOutput( "[TRAIN %d] I got a route! It goes to %s", DATA->train_number, DATA->track[DATA->route.destination.node] );
				//Delay(10);
				begin_route( DATA );
				CommandOutput( "[TRAIN %d] Began Route to %s", DATA->train_number, DATA->track[DATA->route.destination.node] );
				//Delay(10);

				break;
			case TRAIN_ROUTE_RESERVATION_UPDATE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );

				struct train_reservation_update_message *rumsg = (struct train_reservation_update_message *)buffer;

				//Update the local copy of the reservation
				update_reservation( &(DATA->route), rumsg->node, rumsg->edge, rumsg->length_reserved );

				//CommandOutput("[TRAIN %d]	Got Reservation for %s(%d)", DATA->train_number, DATA->track[rumsg->node].name, rumsg->length_reserved);

				DATA->on_secondary_route = FALSE;

				//Reset the switch set delay, so that we set switches based on the new reservation
				//DATA->last_switch_time = ;

				if( DATA->track[rumsg->node].type == NODE_BRANCH || DATA->track[rumsg->node].type == NODE_MERGE) {
					//Set the switch!
					set_upcoming_switches( DATA->track, &DATA->route, &DATA->state.current_pos_guess, INT_MAX, offset_front( DATA ), rumsg->node );
				}

				//set the last time we got a reservation update
				DATA->last_res_update_time = Time();
				DATA->waiting_for_res = 0;

				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );
				break;
		}

		//we have done a lot of processing, get a new time
		//and guess a new position
		DATA->current_time = Time( );
		refresh_state( DATA );
		DATA->state.current_pos_guess = guess_position( DATA );


		//update speeds, stop the train, get more reservation etc
		int pt1 = Time( );
		check_distances( DATA );

		int pt2 = Time( );
		if(pt2 - pt1 > 1){
			CommandOutput( "\x1B[30;41m[TR%d] CHCK_DST [%d]\x1B[0m", DATA->train_number, pt2 - pt1 );
		}

		int time_diff = Time( ) - tinit;
		if( time_diff > 5 ) {
			CommandOutput( "\x1B[30;47m[TR%d] TOOK %d TICKS\x1B[0m", DATA->train_number, time_diff );
		}
	}
}


void check_distances( struct train_data* DATA ) {

	//if we are stopping, or lost, we should not do these checks
	if( DATA->state.current == STATE_STOPPING || DATA->state.current == STATE_STOPPING_REVERSE || DATA->state.lost ) {
		return;
	}


	struct position pos = DATA->state.current_pos_guess;

	//get the distance i have travelled since last sensor hit
	int dist = track_distance( DATA->track, &DATA->state.last_sensor_pos, &pos );

	//how far until the destination
	int destination_distance = INT_MAX;
	if( DATA->route.destination.node != -1 ) {

		destination_distance = primary_route_distance( &DATA->route, &pos, &DATA->dest );

		if( destination_distance < INT_MAX ) {

			//we want the centre of the train to land on the destination, so
			//adjust the distance so that this happens
			destination_distance = destination_distance + (offset_front( DATA ) + offset_rear( DATA )) / 2 - offset_front( DATA );
		}
	}

	//how far will it take to stop
	int stop_dist = current_stop_distance( DATA );


	//should we stop for destination?
	if( stop_dist > destination_distance - CONFIG_TRAIN_DEST_STOP_WINDOW ) {
		if( DATA->state.speed_target != 0 ) {
			CommandOutput( "[TRAIN %d] DEST STOP_DIST[%d] DEST_DIST[%d] STOPWIND[%d] LASTSENSE[%s]", DATA->train_number, stop_dist, destination_distance, CONFIG_TRAIN_DEST_STOP_WINDOW, DATA->track[DATA->state.last_sensor_pos.node].name );
			set_train_speed( DATA, 0 );
		}
		return;
	}

	//what is the minimum track reserved along a path
	int track_dist = route_guaranteed_length( DATA->track, &pos, &DATA->route, DATA->state.last_sensor_pos.node );

	//Do we have enough route? If not, yell at the route server! Yelling is therapeutic, and likely to make
	//people like you!
	if( track_dist < CONFIG_TRAIN_REQUESTED_ROUTE_LENGTH ) {
		notify_position( DATA );
	}

	//do we need to stop to ensure safety?
	if( track_dist < stop_dist + offset_front( DATA ) + CONFIG_TRAIN_SAFE_STOP_BUFFER && DATA->state.last_sensor_pos.node != -1 ) {
		if( DATA->state.speed_target != 0 ) {
			set_train_speed( DATA, 0 );
			CommandOutput( "[TRAIN %d] SAFE STOP_DIST[%d] TRACK_DIST[%d] OFFFRONT[%d] LASTSENSE[%s]", DATA->train_number, stop_dist, track_dist, offset_front( DATA ), DATA->track[DATA->state.last_sensor_pos.node].name );
		}

		if(DATA->state.current == STATE_STOPPED) {
			if(DATA->waiting_for_res){
				if(DATA->current_time > DATA->last_res_update_time + CONFIG_TRAIN_DEADLOCK_DELAY){
					//Free our reservation and ask for only the space we're on now.
					struct position_and_train_message ptmsg;
					ptmsg.message_type = TRAIN_ROUTE_RESERVE_WHERE_I_AM_POS_MESSAGE;
					ptmsg.train_number = DATA->train_number;
					ptmsg.pos = DATA->state.state_change_pos;

					CourierSend(DATA->tid_route_serv, (char*) &ptmsg, sizeof(ptmsg));

					//we are dead locked, request a new path
					struct train_route_request_message rrmsg;
					rrmsg.message_type = TRAIN_ROUTE_REQUEST_MESSAGE;
					rrmsg.pos2.node = DATA->dest.node;
					rrmsg.pos2.offset = DATA->dest.offset;

					rrmsg.pos1.node = DATA->state.current_pos_guess.node;
					rrmsg.pos1.offset = DATA->state.current_pos_guess.offset;
					rrmsg.train_tid = DATA->tid_me;
					rrmsg.train = DATA->train_number;
					rrmsg.requested_route_length = CONFIG_TRAIN_REQUESTED_ROUTE_LENGTH;

					//Pointer for ungodly pointer copy  NEVER DO THIS AGAIN
					//(If you do decide to do this again, at least copy this warning message)
					rrmsg.route = &DATA->route_buffer;
					CommandOutput("[TRAIN %d] Routing to node %s ", DATA->train_number, DATA->track[DATA->dest.node].name, DATA->dest.node);
					CourierSend(DATA->tid_route_serv, (char*) &rrmsg, sizeof(struct train_route_request_message));
					CommandOutput("\x1B[30;42m[TRAIN %d] REROUTED DUE TO DEADLOCK\x1B[0m",DATA->train_number);
					//DATA->waiting_for_res = FALSE;
					DATA->last_res_update_time = DATA->current_time;
					return;
				}
			}
			if(DATA->route.primary_size > 0) {
				int index = primary_route_index(&(DATA->route),0, pos.node);
				if(index >= 0 && index < DATA->route.primary_size && DATA->route.primary[index].reverse_here) {
					reverse_train(DATA);
				}else{
					if(!DATA->waiting_for_res){
						DATA->waiting_for_res = TRUE;
						DATA->last_res_update_time = DATA->current_time;
						CommandOutput("\x1B[30;42m[TRAIN %d] WAITING FOR DEADLOCK TIMEOUTK\x1B[0m",DATA->train_number);
					}
				}
			}
		}
		//CHECK HERE IF WE NEED TO REVERSE:
		//TODO: IMPLEMENT THIS FOR REVERSING AFTER WE HAVE STOPPED
		//AT THE END OF OUR GUARANTEED ROUTE!!!!!
		//MAKE AN ELSE or ELSE IF(NEED TO REVERSE)

		//CALLING REVERSE_ME SHOULD DEAL WITH THE REVERSE

		//NOTE: CHECK A FLAG, OR YOUR STATE TO MAKE SURE YOU ARE NOT ALREADY REVERSING
		//THE TRAIN WILL BREAK OUT OF THIS CASE, AND WILL HAVE TO REWAKE IN ORDER TO COMPLETE
		//THE REVERSE POSSIBLY.
		return;
	}

	if( !DATA->stop_worker_running ) {
		int greatest_stop = greatest_stop_distance( DATA );
		int closest_stop_dest = MIN( (destination_distance - CONFIG_TRAIN_DEST_STOP_WINDOW), (track_dist - CONFIG_TRAIN_SAFE_STOP_BUFFER - offset_front( DATA )) );
		int wakeup_dist = MAX( 0, closest_stop_dest - greatest_stop );
		if( wakeup_dist < CONFIG_SLEEP_WORKER_START_DIST ) {
			//how long untill we need to wakeup?
			int wakeup_time = mot_accel_dist_to_time( jerk( DATA, DATA->state.velocity_last, DATA->state.velocity_target ), DATA->state.velocity_last, DATA->state.velocity_target, wakeup_dist );

			//make a delay worker
			DATA->stop_worker_running = TRUE;
			WorkerDelayUntilWithId( DATA->current_time + wakeup_time, DELAY_STOP_WAKEUP );
		}
	}

	//depending on our current speed, should we change
	if( DATA->state.speed_target == 0 && DATA->dest.node != -1 && DATA->state.current == STATE_STOPPED && DATA->route.destination.node != -1 ) {
		//we should start driving
		CommandOutput( "[TRAIN %d] Starting To Drive ", DATA->train_number );
		set_train_speed( DATA, DATA->calibration.speed_med );
	} else if( DATA->state.speed_target == DATA->calibration.speed_low ) {

	} else if( DATA->state.speed_target == DATA->calibration.speed_med ) {

	} else if( DATA->state.speed_target == DATA->calibration.speed_high ) {

	}

	if( DATA->current_time > DATA->last_lost_check_time + 40 && DATA->sensors.num_sensors > 0) {
		//check if we are lost
		int max_sensor_dist = 0;
		int i = 0;
		for( i = 0; i < DATA->sensors.num_sensors; i++ ) {
			struct position pos;
			pos.offset = 0;
			pos.node = DATA->sensors.nodes[i];
			int sdist = track_distance( DATA->track, &DATA->state.last_sensor_pos, &pos );
			if( sdist == INT_MAX ) {
				CommandOutput( "Unreachable sensor: %d from %d + %d mm", DATA->sensors.nodes[i], DATA->state.last_sensor_pos.node, DATA->state.last_sensor_pos.offset );
			} else {
				max_sensor_dist = MAX( sdist, max_sensor_dist );
			}

		}
		if( dist - CONFIG_TRAIN_SENSOR_ATRIBUTION_DELTA > max_sensor_dist ) {
			CommandOutput( "Overran max_sensor_dist : %d from %d + %d mm", max_sensor_dist, DATA->state.last_sensor_pos.node, DATA->state.last_sensor_pos.offset );
			//we are lost
			CommandOutput("\x1B[43;30m[TR%d] UPDATE LOST 4\x1B[0m", DATA->train_number);
			update_state_lost( DATA );
		}
	}
}


//determine the optimal speed to be travelling at


int optimal_speed( struct train_data* DATA, int safe_dist, int destination_dist ) {

	return 0;
}


void begin_route( struct train_data* DATA ) {
	//notify_release_reservations( DATA );

	DATA->state.last_sensor_pos = DATA->state.current_pos_guess;
	//start watching for the sensors);
	DATA->sensors.num_sensors = predict_sensors_quantum( DATA->track, &DATA->state.current_pos_guess, &DATA->route, DATA->sensors.nodes, DATA->sensors.distances, DATA->sensors.primary, -1 );
	create_sensor_watch( DATA );


	DATA->last_pos_notify_time = -9000;
	//TODO: get reservation

	notify_position( DATA );

	//switch all the switches
	//set_upcoming_switches( DATA->track, &DATA->route, &DATA->state.current_pos_guess, CONFIG_TRAIN_MAX_SWITCH_DISTANCE, offset_front( DATA ) );
}


void dump_calibration( struct train_data* DATA ) {
	int i;

	Printf( COM2, "//Train %d\n", DATA->calibration.number );
	for( i = 0; i < NUM_LOGICAL_SPEEDS; i++ ) {
		Printf( COM2, "d->disttotal[%d] = %d; \t", i, DATA->calibration.disttotal[i] );
		Printf( COM2, "d->timetotal[%d] = %d;\n", i, DATA->calibration.timetotal[i] );
	}
	Printf( COM2, "//End of Train %d\n", DATA->calibration.number );
}

