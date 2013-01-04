#include <userspace.h>

void refresh_state(struct train_data* DATA) {
	//check if enough time has elapsed to even refresh the state
	if (DATA->current_time < DATA->state.state_change_finish_time) {
		return;
	}

	int offset = 0;
	//only states which have a 'finish time' are included
	//others are left out for no action
	switch (DATA->state.current) {
	case STATE_STOPPED_REVERSE:
		DATA->state.current = STATE_ACCELERATING;
		DATA->state.state_change_finish_time = DATA->current_time + mot_time(jerk(DATA, DATA->state.velocity_last,
				DATA->state.velocity_target), DATA->state.velocity_last, DATA->state.velocity_target)
				+ CONFIG_TRAIN_ACCEL_FUDGE_TIME;
		mdi_set_speed(DATA, speed_logical_to_mdi(DATA->state.speed_target));
		break;
	case STATE_STOPPING_REVERSE:
		//reverse the train
		mdi_reverse(DATA);
		//get the resume speed
		int resume_speed = speed_logical_to_mdi(DATA->state.speed_last);

		//calculate how far we have travelled
		offset += mot_distance(jerk(DATA, DATA->state.velocity_last, DATA->state.velocity_target),
				DATA->state.velocity_last, DATA->state.velocity_target, DATA->current_time
						- DATA->state.state_change_start_time);
		DATA->state.state_change_pos.offset += offset;
		//consume the extra offset and move to the current positions node
		traverse_route(DATA->track, &DATA->state.state_change_pos, &DATA->route);

		int node = DATA->state.state_change_pos.node;
		int edge = 0;
		if (DATA->track[node].type == NODE_BRANCH) {
			int status = SwitchStatus(DATA->track[node].num);
			if (status == SWITCH_STRAIGHT) {
				edge = DIR_STRAIGHT;
			} else if (status == SWITCH_CURVED) {
				edge = DIR_CURVED;
			}
		}

		DATA->state.state_change_pos.node = DATA->track[node].edge[edge].dest->reverse->index;
		DATA->state.state_change_pos.offset = DATA->track[node].edge[edge].dist - DATA->state.state_change_pos.offset + DATA->calibration.pickup_len;
		traverse_route(DATA->track,&DATA->state.state_change_pos,&DATA->route);
		switch(DATA->state.direction){
			case DIRECTION_FORWARD:
				DATA->state.direction = DIRECTION_BACKWARD;
				break;
			case DIRECTION_BACKWARD:
				DATA->state.direction = DIRECTION_FORWARD;
				break;
			case DIRECTION_UNKNOWN:
			default:
				DATA->state.direction = DIRECTION_UNKNOWN;
				break;
		}

		DATA->state.last_sensor_pos = DATA->state.state_change_pos;

		//common
		DATA->state.speed_last = 0;
		DATA->state.speed_target = speed_mdi_to_logical(resume_speed, 0);

		DATA->state.velocity_last.d = 0;
		DATA->state.velocity_last.t = 1;
		DATA->state.velocity_target.d = DATA->calibration.disttotal[DATA->state.speed_target];
		DATA->state.velocity_target.t = DATA->calibration.timetotal[DATA->state.speed_target];

		DATA->state.state_change_start_time = DATA->current_time;

		if (resume_speed == 0) {
			DATA->state.current = STATE_STOPPED;
			DATA->state.state_change_finish_time = INT_MAX;
		} else {
			DATA->state.current = STATE_STOPPED_REVERSE;
			DATA->state.state_change_finish_time = DATA->current_time + CONFIG_TRAIN_REVERSE_STOPPED_DELAY;
		}

		//Create a new sensor watch
		struct position new_pos = DATA->state.state_change_pos;
		DATA->sensors.num_sensors = predict_sensors_quantum(DATA->track, &new_pos, &DATA->route, DATA->sensors.nodes,
				DATA->sensors.distances, DATA->sensors.primary, DATA->state.last_sensor_pos.node);

		struct print_buffer pbuff;
		int i;
		ap_init_buff(&pbuff);
		ap_printf(&pbuff, "Creating Sensor Watch (%s + %d) from %s(%d): ", DATA->track[new_pos.node].name, new_pos.offset, DATA->track[DATA->state.last_sensor_pos.node].name, DATA->state.last_sensor_pos.node);
		for (i = 0; i < DATA->sensors.num_sensors; i++) {
			ap_printf(&pbuff, " %s(%d) ", DATA->track[DATA->sensors.nodes[i]].name, DATA->sensors.distances[i]);
		}
		CommandOutput(pbuff.mem);

		create_sensor_watch(DATA);

		DATA->last_pos_notify_time = -9000;
		//TODO: get reservation

		notify_position(DATA);

		break;
	case STATE_STOPPING:
		//calculate how far we have travelled
		offset += mot_distance(jerk(DATA, DATA->state.velocity_last, DATA->state.velocity_target),
				DATA->state.velocity_last, DATA->state.velocity_target, DATA->current_time
						- DATA->state.state_change_start_time);

		DATA->state.state_change_pos.offset += offset;
		//consume the extra offset and move to the current positions node
		traverse_route(DATA->track, &DATA->state.state_change_pos, &DATA->route);

		//now update to the stopped state, we have finished stopping
		DATA->state.current = STATE_STOPPED;
		DATA->state.speed_last = DATA->state.speed_target;
		DATA->state.state_change_start_time = DATA->state.state_change_finish_time;
		DATA->state.state_change_finish_time = INT_MAX;
		DATA->state.velocity_last = DATA->state.velocity_target;

		//are we at a destination?
		if (!DATA->state.lost && DATA->dest.node != -1 && DATA->state.last_sensor_pos.node != -1
				&& !DATA->on_secondary_route) {
			int destination_distance = primary_route_distance(&DATA->route, &DATA->state.last_sensor_pos, &DATA->dest);
			struct position my_pos = guess_position(DATA);
			int my_distance = track_distance(DATA->track, &DATA->state.last_sensor_pos, &my_pos);

			if (ABS( destination_distance - my_distance ) < CONFIG_TRAIN_DEST_ARRIVE_WINDOW) {
				//we are at the destination!
				destination_arrive(DATA);
			}
		}

		break;
	case STATE_ACCELERATING:
		//calculate how far we have travelled
		offset += mot_distance(jerk(DATA, DATA->state.velocity_last, DATA->state.velocity_target),
				DATA->state.velocity_last, DATA->state.velocity_target, DATA->current_time
						- DATA->state.state_change_start_time);

		DATA->state.state_change_pos.offset += offset;
		//consume the extra offset and move to the current positions node
		traverse_route(DATA->track, &DATA->state.state_change_pos, &DATA->route);

		//now update to the normal state, we have finished accelerating
		DATA->state.current = STATE_NORMAL;
		DATA->state.speed_last = DATA->state.speed_target;
		DATA->state.state_change_start_time = DATA->state.state_change_finish_time;
		DATA->state.state_change_finish_time = INT_MAX;
		DATA->state.velocity_last = DATA->state.velocity_target;
		break;
	}
}

void destination_arrive(struct train_data* DATA) {
	//TODO: send a message saying we arrived
	CommandOutput("[TR%d] ARRIVED AT DESTINATION", DATA->train_number);
	//clear the destination
	DATA->dest.node = -1;

	//clear the route
	init_route(&DATA->route);

	//send the arrive msg
	struct two_number_message msg;
	msg.message_type = GAME_FINISHED_ROUTE_MESSAGE;
	msg.num1 = DATA->train_number;
	CourierSend(WhoIs("gameAI"), (char*) &msg, sizeof(msg));
}

//update the train state due to a sensor hit


void update_state_from_sensor(struct train_data* DATA, int sensor_node, int sensor_time) {
	//if the sensor node is -1, we are lost
	if (sensor_node < 0) {
		CommandOutput("\x1B[43;30m[TR%d] UPDATE LOST 1\x1B[0m", DATA->train_number);
		update_state_lost(DATA);
		return;
	}
	int sensor_index = -1;
	//decide if we should ignore this sensor, only if we knew where we were
	if (!DATA->state.lost) {
		//check if the sensor is in our attribution list
		int i = 0;
		for (i = 0; i < DATA->sensors.num_sensors; i++) {
			if (DATA->sensors.nodes[i] == sensor_node) {
				sensor_index = i;
				break;
			}
		}
		if (sensor_index == -1) {
			//we are not waiting on this sensor, ignore it
			//TODO: inform the attribution server we ignored the sensor
			CommandOutput("\x1B[37m[TR%d] IGNORED SENSOR %s\x1B[0m", DATA->train_number, DATA->track[sensor_node].name);
			return;
		}

		//where do we think we are
		struct position guessed_pos = DATA->state.current_pos_guess;

		//how far is our position past the last sensor
		int guessed_dist = track_distance(DATA->track, &DATA->state.last_sensor_pos, &guessed_pos);

		//how far is this sensor from our last
		struct position cur_pos;
		cur_pos.node = sensor_node;
		cur_pos.offset = 0;
		int sensor_dist = DATA->sensors.distances[sensor_index];

		//how far are we off
		int error = guessed_dist - sensor_dist;
		int delta = ABS( error );

		CommandOutput("[TR%d] GUESS[%d] SENS[%d] ERR[%d]", DATA->train_number, guessed_dist, sensor_dist, error);

		if (delta > CONFIG_TRAIN_SENSOR_ATRIBUTION_DELTA || DATA->state.current == STATE_STOPPED || DATA->state.current
				== STATE_STOPPED_REVERSE) {
			//we are not in the window, ignore the sensor
			//TODO: inform the attribution server we ignored the sensor
			CommandOutput("\x1B[31m[TR%d] IGNORED SENSOR %s\x1B[0m", DATA->train_number, DATA->track[sensor_node].name);
			return;
		}

		//we should update the error
		DATA->error_last = error;
		if (delta > ABS( DATA->error_max )) {
			DATA->error_max = error;
		}

		CommandOutput("\x1B[32m[TR%d] DIDNT IGNORE SENSOR %s\x1B[0m", DATA->train_number, DATA->track[sensor_node].name);

		//check if we should update calibration
		/*
		 if( DATA->state.current == STATE_NORMAL ) {
		 if( !DATA->state.changed_speed ) {
		 int dt = sensor_time - DATA->state.last_sensor_time;
		 //update calibration
		 DATA->calibration.disttotal[DATA->state.speed_target] = (DATA->calibration.disttotal[DATA->state.speed_target] * 9 + 1 * sensor_dist) / 10;
		 DATA->calibration.timetotal[DATA->state.speed_target] = (DATA->calibration.timetotal[DATA->state.speed_target] * 9 + 1 * dt) / 10;

		 while( DATA->calibration.disttotal[DATA->state.speed_target] > 0xFFFF || DATA->calibration.timetotal[DATA->state.speed_target] > 0xFFFF ) {
		 DATA->calibration.disttotal[DATA->state.speed_target] >>= 1;
		 DATA->calibration.timetotal[DATA->state.speed_target] >>= 1;
		 }
		 }
		 DATA->state.changed_speed = FALSE;
		 }*/
	}

	int dt = sensor_time - DATA->state.state_change_start_time;

	//how far we should have travelled in this time
	int dist = 0;

	switch (DATA->state.current) {
	case STATE_NORMAL:
		//just put the train at the sensor
		DATA->state.state_change_start_time = sensor_time;
		dist = 0;
		break;
	case STATE_STOPPING:
	case STATE_STOPPING_REVERSE:
	case STATE_ACCELERATING:
		dist = mot_distance(jerk(DATA, DATA->state.velocity_last, DATA->state.velocity_target),
				DATA->state.velocity_last, DATA->state.velocity_target, dt);
		break;
	}



	//set the position based on the dist calculation
	DATA->state.state_change_pos.node = sensor_node;
	DATA->state.state_change_pos.offset = 0 - dist;

	//update the last sensor
	DATA->state.last_sensor_pos.node = sensor_node;
	DATA->state.last_sensor_pos.offset = 0;

	//set our guessed position
	DATA->state.current_pos_guess.node = sensor_node;
	DATA->state.current_pos_guess.offset = 0;

	if (DATA->state.lost && DATA->dest.node != -1) {
		struct two_number_message twomsg;
		twomsg.message_type = TRAIN_ROUTE_RESERVE_WHERE_I_AM_MESSAGE;
		twomsg.num1 = DATA->train_number;
		twomsg.num2 = sensor_node;

		CommandOutput("[TRAIN %d] Found myself at %s, can I haz route?", DATA->train_number,
				DATA->track[sensor_node].name, DATA->dest.node);
		CourierSend(DATA->tid_route_serv, (char*) &twomsg, sizeof(twomsg));

		//we hit a secondary sensor, re route
		//get a new route
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

		CommandOutput("[TRAIN %d] Routing to node %s ", DATA->train_number, DATA->track[DATA->dest.node].name,
				DATA->dest.node);
		CourierSend(DATA->tid_route_serv, (char*) &rrmsg, sizeof(struct train_route_request_message));
	} else if(DATA->state.lost && DATA->dest.node < 0 && sensor_node >= 0) {
		struct two_number_message twomsg;
		twomsg.message_type = TRAIN_ROUTE_RESERVE_WHERE_I_AM_MESSAGE;
		twomsg.num1 = DATA->train_number;
		twomsg.num2 = sensor_node;

		CommandOutput("[TRAIN %d] Found myself at %s, can I haz route?", DATA->train_number,
				DATA->track[sensor_node].name, DATA->dest.node);
		CourierSend(DATA->tid_route_serv, (char*) &twomsg, sizeof(twomsg));
	} else if (DATA->sensors.primary[sensor_index] != TRUE) {
		struct two_number_message twomsg;
		twomsg.message_type = TRAIN_ROUTE_RESERVE_WHERE_I_AM_MESSAGE;
		twomsg.num1 = DATA->train_number;
		twomsg.num2 = sensor_node;

		CommandOutput("[TRAIN %d] Found myself at %s, can I haz route?", DATA->train_number,
				DATA->track[sensor_node].name, DATA->dest.node);
		CourierSend(DATA->tid_route_serv, (char*) &twomsg, sizeof(twomsg));

		//get a new route
		struct train_route_request_message rrmsg;
		rrmsg.message_type = TRAIN_ROUTE_REQUEST_MESSAGE;
		rrmsg.pos2.node = DATA->dest.node;
		rrmsg.pos2.offset = DATA->dest.offset;
		rrmsg.requested_route_length = CONFIG_TRAIN_REQUESTED_ROUTE_LENGTH;

		DATA->on_secondary_route = TRUE;
		DATA->route.destination.node = -1;

		rrmsg.pos1.node = sensor_node;
		rrmsg.pos1.offset = 0;
		rrmsg.train_tid = DATA->tid_me;
		rrmsg.train = DATA->train_number;

		//Pointer for ungodly pointer copy  NEVER DO THIS AGAIN
		//(If you do decide to do this again, at least copy this warning message)
		rrmsg.route = &DATA->route_buffer;

		CommandOutput("[TRAIN %d] Hit Secondary Sensor, Rerouting to %s ", DATA->train_number,
				DATA->track[DATA->dest.node].name, DATA->dest.node);
		CourierSend(DATA->tid_route_serv, (char*) &rrmsg, sizeof(struct train_route_request_message));
	}

	if (!DATA->state.lost && !DATA->on_secondary_route && DATA->route.primary_size > 0) {
		//recalculate and update the sensor watch
		struct position new_pos = DATA->state.current_pos_guess;
		DATA->sensors.num_sensors = predict_sensors_quantum(DATA->track, &new_pos, &DATA->route, DATA->sensors.nodes,
				DATA->sensors.distances, DATA->sensors.primary, DATA->state.last_sensor_pos.node);
		create_sensor_watch(DATA);

		//Record that we saw this sensor
		int index = primary_route_index(&DATA->route, 0, sensor_node);
		if(index >= 0) {
			DATA->route.primary[index].passed = TRUE;
		}
	}

	notify_position(DATA);
	notify_release_reservations(DATA);

	//we are not lost
	DATA->state.lost = FALSE;

}

/* sensor interpreter functions */
void create_unatrib_sensor_watch(struct train_data* DATA) {
	struct sensor_watch_message msg;
	msg.message_type = SENSOR_WATCH_MESSAGE;
	msg.type = SENSOR_WATCH_TYPE_UNATTRIBUTED_SENSOR;
	msg.id = UNATTRIBUTED_SENSOR_WATCH;
	msg.tid = DATA->tid_me;

	CourierSend(DATA->tid_sensor_interp, (char *) &msg, sizeof(msg));
}

void create_sensor_watch(struct train_data* DATA) {
	struct sensor_watch_message msg;
	msg.message_type = SENSOR_WATCH_MESSAGE;
	msg.type = SENSOR_WATCH_TYPE_SENSOR;
	msg.id = NORMAL_SENSOR_WATCH;
	msg.tid = DATA->tid_me;

	//set the timeout
	msg.timeout = 10000;

	//copy the sensor information to the message
	msg.num_sensors = DATA->sensors.num_sensors;
	mem_copy((char *) DATA->sensors.nodes, (char*) msg.sensors, sizeof(int) * DATA->sensors.num_sensors);

	CourierSend(DATA->tid_sensor_interp, (char *) &msg, sizeof(msg));

	struct print_buffer pbuff;
	int i;
	ap_init_buff(&pbuff);
	ap_printf(&pbuff, "Creating Sensor Watch: ");
	for (i = 0; i < DATA->sensors.num_sensors; i++) {
		ap_printf(&pbuff, " %s(%d) ", DATA->track[DATA->sensors.nodes[i]].name, DATA->sensors.distances[i]);
	}
	CommandOutput(pbuff.mem);
}

struct position guess_position(struct train_data* DATA) {
	struct position ret_pos;
	ret_pos.node = DATA->state.state_change_pos.node;
	ret_pos.offset = DATA->state.state_change_pos.offset;

	//calculate the elapsed time
	int dt = DATA->current_time - DATA->state.state_change_start_time;
	if (dt <= 0) {
		return ret_pos;
	}

	//how far past the state change have we travelled
	int offset = 0;

	switch (DATA->state.current) {
	case STATE_STOPPED_REVERSE:
	case STATE_STOPPED:
		//we are stopped, we shouldn't have moved
		offset = 0;
		break;
	case STATE_NORMAL:
	case STATE_STOPPING:
	case STATE_STOPPING_REVERSE:
	case STATE_ACCELERATING:
		offset = mot_distance(jerk(DATA, DATA->state.velocity_last, DATA->state.velocity_target),
				DATA->state.velocity_last, DATA->state.velocity_target, dt);
		break;
	}

	//we know how far we have travelled extra, add it to the offset, then
	//traverse the route
	ret_pos.offset += offset;
	int sensors_passed = 0;
	if (!DATA->on_secondary_route && DATA->route.primary_size > 0) {
		sensors_passed += traverse_route(DATA->track, &ret_pos, &DATA->route);
	}

	/*if( sensors_passed >= 2 ) {
	 ret_pos.node = -1;
	 ret_pos.offset = 0;
	 update_state_lost( DATA );
	 CommandOutput( "Oh, shit, we're missing sensors!!!" );
	 }*/

	if(passed_too_many_sensors(DATA->track, &DATA->route, &ret_pos)) {
		CommandOutput("\x1B[43;30m[TR%d] UPDATE LOST 2\x1B[0m", DATA->train_number);
		update_state_lost(DATA);
	}

	return ret_pos;
}

int current_stop_distance(struct train_data* DATA) {
	//get the current velocity
	int dt = DATA->current_time - DATA->state.state_change_start_time;
	struct velocity vcur = mot_veloc(jerk(DATA, DATA->state.velocity_last, DATA->state.velocity_target),
			DATA->state.velocity_last, DATA->state.velocity_target, dt);
	//calculate stop distance at current velocity
	struct velocity vstop;
	vstop.d = 0;
	vstop.t = 1;
	return mot_distance(jerk(DATA, vcur, vstop), vcur, vstop, mot_time(jerk(DATA, vcur, vstop), vcur, vstop));
}

int stop_distance(struct train_data* DATA, struct velocity vi) {
	struct velocity vstop;
	vstop.d = 0;
	vstop.t = 1;

	return mot_distance(jerk(DATA, vi, vstop), vi, vstop, mot_time(jerk(DATA, vi, vstop), vi, vstop));
}

int greatest_stop_distance(struct train_data* DATA) {
	int current = current_stop_distance(DATA);

	//calculate stop distance at current velocity
	struct velocity vstop;
	vstop.d = 0;
	vstop.t = 1;
	int final = mot_distance(jerk(DATA, DATA->state.velocity_target, vstop), DATA->state.velocity_target, vstop,
			mot_time(jerk(DATA, DATA->state.velocity_target, vstop), DATA->state.velocity_target, vstop));
	return MAX( current, final );
}

void update_state_lost(struct train_data* DATA) {
	CommandOutput("[TR%d] WAS LOST", DATA->train_number);
	DATA->state.lost = TRUE;
	DATA->state.state_change_pos.node = -1;
	DATA->state.state_change_pos.offset = 0;

	DATA->state.last_sensor_pos.node = -1;
	DATA->state.last_sensor_pos.offset = 0;

	init_route(&DATA->route);

	//wait for a sensor which is unattributed
	//TODO: we need to make sure two trains aren't lost at once
	//or this will fail
	create_unatrib_sensor_watch(DATA);
	//clear the sensors we are watching
	DATA->sensors.num_sensors = 0;

	//start the train moving slowly, we need to find ourselves
	set_train_speed(DATA, DATA->calibration.speed_low);
}

void reverse_train(struct train_data* DATA) {
	//set my speed to zero
	CommandOutput("Reversing Train");
	set_train_speed(DATA, 0);
	if (DATA->state.state_change_finish_time == INT_MAX) {
		DATA->state.state_change_finish_time = DATA->current_time;
		DATA->state.speed_last = 0;
	}
	DATA->state.current = STATE_STOPPING_REVERSE;
	refresh_state(DATA);
}

void set_train_speed(struct train_data* DATA, int speed) {

	if (DATA->state.current == STATE_STOPPING_REVERSE || DATA->state.current == STATE_STOPPED_REVERSE) {
		//we are stopping to reverse, we shouldn't set the speed, just update
		DATA->state.speed_last = speed_mdi_to_logical(speed, 0);
		return;
	}

	if (speed == speed_logical_to_mdi(DATA->state.speed_target) && DATA->state.current != STATE_STOPPING_REVERSE
			&& DATA->state.current != STATE_STOPPED_REVERSE) {
		//we are already at that speed, just exit
		return;
	}

	//we have changed speed since last sensor
	DATA->state.changed_speed = TRUE;

	//set the speed
	mdi_set_speed(DATA, speed);

	//calculate the elapsed time since the last state change
	int dt = DATA->current_time - DATA->state.state_change_start_time;

	//update our position
	DATA->state.state_change_pos.offset += mot_distance(jerk(DATA, DATA->state.velocity_last,
			DATA->state.velocity_target), DATA->state.velocity_last, DATA->state.velocity_target, dt);

	//update to the current velocity
	DATA->state.velocity_last = mot_veloc(jerk(DATA, DATA->state.velocity_last, DATA->state.velocity_target),
			DATA->state.velocity_last, DATA->state.velocity_target, dt);

	//save the current speed (may be incorrect, but that is okay)
	DATA->state.speed_last = DATA->state.speed_target;

	if (speed == 0) {
		DATA->state.current = STATE_STOPPING;
		DATA->state.speed_target = 0;
	} else {
		DATA->state.current = STATE_ACCELERATING;
		DATA->state.speed_target = speed_mdi_to_logical(speed, DATA->state.speed_target);
	}

	//set the time
	DATA->state.state_change_start_time = DATA->current_time;

	//set the velocity target
	DATA->state.velocity_target.d = DATA->calibration.disttotal[DATA->state.speed_target];
	DATA->state.velocity_target.t = DATA->calibration.timetotal[DATA->state.speed_target];

	//caluclate the state change finish time
	DATA->state.state_change_finish_time = DATA->current_time + mot_time(jerk(DATA, DATA->state.velocity_last,
			DATA->state.velocity_target), DATA->state.velocity_last, DATA->state.velocity_target)
			+ CONFIG_TRAIN_ACCEL_FUDGE_TIME;

	if (!DATA->on_secondary_route && DATA->route.primary_size > 0) {
		//consume the offset
		traverse_route(DATA->track, &DATA->state.state_change_pos, &DATA->route);
	}

	//place our guessed position at this place
	DATA->state.current_pos_guess = DATA->state.state_change_pos;

	//request a state change update incase the acceleration is instant (same speed etc.)
	refresh_state(DATA);
}

int jerk(struct train_data* DATA, struct velocity vi, struct velocity vf) {
	if (((double) vi.d / vi.t) < ((double) vf.d / vf.t)) {
		return DATA->calibration.jerk_const_up;
	}
	return DATA->calibration.jerk_const_down;
}

/* mdi server msg functions */
void mdi_set_speed(struct train_data* DATA, int speed) {
	//tell the mdi to change my speed
	struct two_number_message msg;
	msg.message_type = TRAIN_CTRL_MESSAGE;
	msg.num1 = DATA->train_number;
	msg.num2 = speed;
	CourierSend(DATA->tid_mdi, (char *) &msg, sizeof(msg));
}

void mdi_reverse(struct train_data* DATA) {
	//tell the mdi to change my speed
	struct two_number_message msg;
	msg.message_type = TRAIN_REVERSE_MESSAGE;
	msg.num1 = DATA->train_number;
	msg.num2 = 0;
	CourierSend(DATA->tid_mdi, (char *) &msg, sizeof(msg));
}
/* end of mdi server msg functions */

//notify interested tasks (such as the route server) about our current position


void notify_position(struct train_data * DATA) {
	struct train_reservation_request_message msg;
	if (DATA->route.primary_size > 0 && DATA->last_pos_notify_time < DATA->current_time
			- CONFIG_TRAIN_NOTIFY_POSITION_INTERVAL) {
		//CommandOutput("Sending reservation request from train %d (tid %d)", DATA->train_number, MyTid());

		msg.message_type = TRAIN_ROUTE_RESERVATION_REQUEST_MESSAGE;
		msg.pos = DATA->state.current_pos_guess;
		msg.requested_route_length = CONFIG_TRAIN_REQUESTED_ROUTE_LENGTH;
		msg.train = DATA->train_number;
		CourierSend(DATA->tid_route_serv, (char *) &msg, sizeof(msg));
		DATA->last_pos_notify_time = DATA->current_time;
	}
}

//msg the route server that we can release reservations


void notify_release_reservations(struct train_data* DATA) {
	struct two_number_message msg;
	struct position back_pos = DATA->state.last_sensor_pos;
	back_pos.offset -= offset_rear(DATA);
	traverse_route(DATA->track, &back_pos, &DATA->route);
	if (DATA->route.primary_size > 0 && !DATA->state.lost && back_pos.node >= 0 && back_pos.node >= 0 && back_pos.offset >= 0) {
		//CommandOutput( "Sending reservation release request from train %d (tid %d)", DATA->train_number, MyTid( ) );
		msg.message_type = TRAIN_ROUTE_FREE_RESERVATION_MESSAGE;

		msg.num1 = DATA->train_number;
		msg.num2 = back_pos.node;
		CourierSend(DATA->tid_route_serv, (char *) &msg, sizeof(msg));
		route_release_reservations(&DATA->route, &back_pos);
		//TODO: update my reservations
	}
}

