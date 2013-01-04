#include <userspace.h>

//Initialize route structure
//WORKING
void init_route(struct train_route * r) {
	int i, j;
	for (i = 0; i < CONFIG_MAX_ROUTE_SIZE + 1; i++) {
		r->primary[i].node = -1;
		r->primary[i].distance = INT_MAX;
		r->primary[i].length_reserved = 0;
		r->primary[i].secondary_route_index = -1;
		r->primary[i].edge = 0;
		r->primary[i].reverse_here = 0;
		r->primary[i].passed = 0;
	}
	for (i = 0; i < CONFIG_MAX_NUM_SECONDARY_ROUTES + 1; i++) {
		for (j = 0; j < CONFIG_MAX_SECONDARY_ROUTE_SIZE; j++) {
			r->secondary[i][j].node = -1;
			r->secondary[i][j].distance = INT_MAX;
			r->secondary[i][j].length_reserved = 0;
			r->secondary[i][j].secondary_route_index = -1;
			r->secondary[i][j].edge = 0;
			r->secondary[i][j].reverse_here = 0;
			r->secondary[i][j].passed = 0;
		}
		r->secondary_size[i] = 0;
	}
	r->num_secondary_routes = 0;
	r->primary_size = 0;
	r->destination.node = -1;
	r->destination.offset = 0;
}

void copy_route(volatile struct train_route * source, volatile struct train_route * dest) {
	int i = 0; int j = 0;
	dest->primary_size = source->primary_size;
	dest->destination.node = source->destination.node;
	dest->destination.offset = source->destination.offset;
	dest->num_secondary_routes = source->num_secondary_routes;
	for(i = 0; i < source->primary_size && i < CONFIG_MAX_ROUTE_SIZE; i++) {
		dest->primary[i].distance = source->primary[i].distance;
		dest->primary[i].edge = source->primary[i].edge;
		dest->primary[i].length_reserved = source->primary[i].length_reserved;
		dest->primary[i].node = source->primary[i].node;
		dest->primary[i].secondary_route_index = source->primary[i].secondary_route_index;
		dest->primary[i].reverse_here = source->primary[i].reverse_here;
		dest->primary[i].passed = source->primary[i].passed;
	}
	for(i = 0; i < source->num_secondary_routes && i < CONFIG_MAX_NUM_SECONDARY_ROUTES; i++) {
		dest->secondary_size[i] = source->secondary_size[i];
		for(j = 0; j < source->secondary_size[i] && j < CONFIG_MAX_SECONDARY_ROUTE_SIZE; j++) {
			dest->secondary[i][j].distance = source->secondary[i][j].distance;
			dest->secondary[i][j].edge = source->secondary[i][j].edge;
			dest->secondary[i][j].length_reserved = source->secondary[i][j].length_reserved;
			dest->secondary[i][j].node = source->secondary[i][j].node;
			dest->secondary[i][j].secondary_route_index = source->secondary[i][j].secondary_route_index;
			dest->secondary[i][j].reverse_here = source->secondary[i][j].reverse_here;
			dest->secondary[i][j].passed = source->secondary[i][j].passed;
		}
	}
}


/*
 *Distance functions
 */

//Distance from node1 to node2 on the track
//TESTED
int track_distance(struct track_node *track, struct position *pos1, struct position *pos2) {
	//create the buffer for input characters
	int nodequeue_mem[TRACK_MAX];
	int distance[TRACK_MAX];
	int curnode;
	int i;
	int pathdistance;
	int node1 = pos1->node, node2 = pos2->node;
	int nextnode = -1;

	struct int_buffer nodequeue;
	intbuffer_init(&nodequeue, nodequeue_mem, TRACK_MAX );
	intbuffer_push(&nodequeue, node1);
	curnode = -1;

	if(pos1->node < 0 || pos2->node < 0 || pos1->node >= TRACK_MAX || pos2->node >= TRACK_MAX) {
		return INT_MAX;
	}

	if(pos1->node == pos2->node) {
		return pos2->offset - pos1->offset;
	}

	for (i = 0; i < TRACK_MAX; i++) {
		distance[i] = INT_MAX;
	}

	distance[node1] = 0;

	while (nodequeue.state != INTBUFF_EMPTY) {
		curnode = (int) intbuffer_pop(&nodequeue);

		//CommandOutput("Visiting node %d", curnode);
		if (curnode == node2) {
			return distance[curnode] + pos2->offset - pos1->offset;
		}
		if (track[curnode].type != NODE_EXIT) {
			pathdistance = distance[curnode] + track[curnode].edge[0].dist;
			nextnode = track[curnode].edge[0].dest->index;
			if (distance[nextnode] > pathdistance) {
				distance[nextnode] = pathdistance;
				intbuffer_push(&nodequeue, nextnode);
			}
		}
		if (track[curnode].type == NODE_BRANCH) {
			pathdistance = distance[curnode] + track[curnode].edge[1].dist;
			nextnode = track[curnode].edge[1].dest->index;
			if (distance[nextnode] > pathdistance) {
				distance[nextnode] = pathdistance;
				intbuffer_push(&nodequeue, nextnode);
			}
		}
	}

	return INT_MAX;

}

//Index of the node along the primary route
//TESTED
int primary_route_index(struct train_route*route, int start, int node) {
	int index;
	//Find the route index of the given position
	for (index = start; index < route->primary_size && route->primary[index].node != node; index++)
		;
	if (index >= route->primary_size) {
		//We're not on the route!
		return -1;
	}
	return index;
}

//Index of the node along the secondary route
int secondary_route_index(struct train_route * route, int secondary_index, int start, int node) {
	int index = 0;
	//Find the route index of the given position
	for (index = 0; index < route->secondary_size[secondary_index] && route->secondary[secondary_index][index].node
			!= node; index++)
		;
	if (index >= route->secondary_size[secondary_index]) {
		//We're not on the route!
		return -1;
	}
	return index;
}

//Distance between two nodes along the primary route
//TESTED
int primary_route_distance(struct train_route *route, struct position *pos1, struct position *pos2) {
	int i = 0;
	int index1 = primary_route_index(route, 0, pos1->node);
	int index2 = primary_route_index(route, 0, pos2->node);
	if (index1 < 0 || index2 < 0) {
		//Either or both points aren't on the route
		return INT_MAX;
	}
	for (i = index1; i < index2; i++) {
		if (route->primary[i].reverse_here) {
			//We reverse between the two given points: Distance is undefined
			return INT_MAX;
		}
	}
	for (i = index2; i < index1; i++) {
		if (route->primary[i].reverse_here) {
			//We reverse between the two given points: Distance is undefined
			return INT_MAX;
		}
	}
	return route->primary[index2].distance - pos1->offset - route->primary[index1].distance + pos2->offset;
}

//Update position based on how far we've travelled from the last node
//TESTED
int traverse_route(struct track_node *track, struct position * pos, struct train_route * route) {
	int dist = 0, i = 0, initial_node = pos->node;
	if (route->primary_size == 0) {
		//We have no fucking route, traverse the track yourself
		return 0;
	}

	//Set i to the position of the current node on the route
	for (i = 0; i < route->primary_size && route->primary[i].node != pos->node; i++)
		;
	if (i >= route->primary_size) {
		pos->node = -1;
		pos->offset = 0;
		return 0;
	}

	if(pos->offset < 0) {
		i--;
		//While the offset from the current node is greater than the next edge length, we
		while (i >= 0) {
			if (route->primary[i].edge == 0) {
				//Route follows edge 0
				dist = track[route->primary[i].node].edge[0].dist;
			} else if (track[route->primary[i].node].type == NODE_BRANCH && route->primary[i].edge == 1) {
				//Route follows edge 1
				dist = track[route->primary[i].node].edge[1].dist;
			}
			if (pos->offset > 0) {
				return 0;
			} else {
				pos->offset += dist;
				pos->node = route->primary[i].node;
			}

			if(route->primary[i].reverse_here) {
				return 0;
			}
			i--;
		}
		return 0;
	} else {
		//While the offset from the current node is greater than the next edge length, we
		while (i < route->primary_size) {
			if (route->primary[i].edge == 0) {
				//Route follows edge 0
				dist = track[route->primary[i].node].edge[0].dist;
			} else if (track[route->primary[i].node].type == NODE_BRANCH && route->primary[i].edge == 1) {
				//Route follows edge 1
				dist = track[route->primary[i].node].edge[1].dist;
			}
			if (pos->offset < dist) {
				pos->node = route->primary[i].node;
				return 0;
			} else {
				pos->offset -= dist;
			}

			if(route->primary[i].reverse_here) {
				return 0;
			}
			i++;
		}
		//Oh shit, we passed an exit node
		if (track[pos->node].type == NODE_EXIT) {
			pos->offset = 0;
		}
		return 0;
	}

}


/*
 * Train location functions
 */

//Edge we expect to take going forward from each node on the track
int track_expected_edge(struct track_node * track, int node) {
	int swstatus;
	if(track[node].type == NODE_BRANCH) {
		swstatus = SwitchStatus(track[node].num);
		if(swstatus == SWITCH_CURVED) {
			return DIR_CURVED;
		} else if(swstatus == SWITCH_CURVED) {
			return DIR_STRAIGHT;
		}
	}
	return DIR_STRAIGHT;
}

//Given a node and distance in front of it, traverse the track in a forward direction until we
//find where the corresponding position is on the track
void traverse_track(struct track_node * track, struct position * pos) {
	int next_dist = 0, next_node;
	int expected_edge;
	if(pos->node < 0 || track[pos->node].type == NODE_EXIT) {
		return;
	}
	expected_edge = track_expected_edge(track, pos->node);
	next_dist = track[pos->node].edge[expected_edge].dist;
	next_node = track[pos->node].edge[expected_edge].dest->index;
	while(pos->offset > next_dist) {
		pos->node = next_node;
		pos->offset -= next_dist;
		if(track[pos->node].type == NODE_EXIT) {
			return;
		}
		expected_edge = track_expected_edge(track, pos->node);
		next_dist = track[pos->node].edge[expected_edge].dist;
		next_node = track[pos->node].edge[expected_edge].dest->index;
	}
}

//Given a node and distance in front of it, traverse the track in a forward direction until we
//find where the corresponding position is on the track
void traverse_track_edge(struct track_node * track, struct position * pos) {
	int next_dist = 0, next_node;
	int expected_edge;
	if(pos->node < 0 || track[pos->node].type == NODE_EXIT) {
		return;
	}
	expected_edge = pos->edge;//track_expected_edge(track, pos->node);
	next_dist = track[pos->node].edge[expected_edge].dist;
	next_node = track[pos->node].edge[expected_edge].dest->index;
	while(pos->offset > next_dist) {
		pos->node = next_node;
		pos->offset -= next_dist;
		if(track[pos->node].type == NODE_EXIT) {
			return;
		}
		expected_edge = track_expected_edge(track, pos->node);
		next_dist = track[pos->node].edge[expected_edge].dist;
		next_node = track[pos->node].edge[expected_edge].dest->index;
	}
}

//Find the position of the front of the train, given a position and it's length in front of it
struct position train_front(struct track_node * track, struct position * pos, int front_length) {
	struct position front_pos = *pos;
	front_pos.offset += front_length;
	traverse_track(track, &front_pos);
	return front_pos;
}

//Find the reverse of the current position (node + offset)
struct position reverse_position(struct track_node * track, struct position * pos) {
	struct position reverse;

	if(pos->node < 0) {
		return *pos;
	}
	if(track[pos->node].type == NODE_EXIT) {
		reverse.node = track[pos->node].reverse->index;
		reverse.offset = -pos->offset;
		reverse.edge = 0;
	} else {
		if(track[pos->node].type == NODE_BRANCH) {
			if(track[pos->node].edge[0].dest->reverse->index);
		}
		reverse.node = track[pos->node].edge[track_expected_edge(track, pos->node)].dest->reverse->index;
		if(track[reverse.node].type == NODE_EXIT) {
			reverse.edge = 0;
		} else if(track[reverse.node].edge[0].dest->reverse->index == pos->node) {
			reverse.edge = 0;
		} else if(track[reverse.node].edge[1].dest->reverse->index == pos->node) {
			reverse.edge = 1;
		} else {
			reverse.edge = 0;
		}
		reverse.offset = track[reverse.node].edge[reverse.edge].dist - pos->offset;
	}
	return reverse;
}

//Find the position of the back of the train, given a position and it's length behind
struct position train_back(struct track_node * track, struct position * pos, int back_length) {
	struct position back_pos = *pos;
	//printf("pos %s + %d\n", track[pos->node].name, pos->offset);
	back_pos = reverse_position(track, pos);
	//printf("back_pos %s + %d\n", track[back_pos.node].name, back_pos.offset);
	back_pos.offset += back_length;
	//printf("back_pos %s + %d\n", track[back_pos.node].name, back_pos.offset);
	traverse_track_edge(track, &back_pos);
	//printf("back_pos %s + %d\n", track[back_pos.node].name, back_pos.offset);
	back_pos = reverse_position(track, &back_pos);
	//printf("back_pos %s + %d\n", track[back_pos.node].name, back_pos.offset);
	return back_pos;
}

//Find the position of the back of the train, given a position and it's length behind
struct position train_back_on_route(struct train_route * route, struct track_node * track, struct position * pos, int back_length) {
	struct position lpos = *pos;
	lpos.offset -= back_length;
	traverse_route(track,&lpos, route);
	return lpos;
}


int passed_too_many_sensors(struct track_node * track, struct train_route * route, struct position * pos) {
	int node = pos->node;
	int count = 0;
	int i;
	for(i = 0; i < route->primary_size; i++) {
		if(pos->offset < CONFIG_TRAIN_SENSOR_ATRIBUTION_DELTA && route->primary[i].node == pos->node) {
			return FALSE;
		}
		if(track[route->primary[i].node].type == NODE_SENSOR) {
			if(route->primary[i].passed) {
				count = 0;
			} else {
				if(i != 0 && !route->primary[i - 1].reverse_here) {
					count++;
				}
			}
		}
		if(count >= 2) {
			return TRUE;
		}
		if(route->primary[i].node == pos->node) {
			return FALSE;
		}
	}
	return FALSE;
}

/*
 * Switch functions
 */

//Set switches corresponding to a particular train route
//TESTED
void set_upcoming_switches(struct track_node *track, struct train_route * route, struct position * pos, int max_dist,
		int min_dist, int switch_to_set) {
	int i = 0, k = 0, dist = 0, secondary_index = 0;

	//TODO: Improve secondary route handling (currently, only sets switches before going onto a secondary route)
	//	(we may need to improve this)

	//Set i to the position of the current node on the route
	for (i = 0; i < route->primary_size && route->primary[i].node != pos->node; i++)
		;

	if (i >= route->primary_size) {
		//We're OUT of ROUTE!!!
		return;
	}

	dist = -pos->offset + track[route->primary[i].node].edge[route->primary[i].edge].dist;
	i++;

	//Set switches along primary route (in case switch changes accidentally wipe out changes for secondary route)
	for (; (i < route->primary_size) && (dist <= max_dist); i++) {
		if (route->primary[i].length_reserved <= 0) {
			//Stop setting switches when we run out of route
			return;
		}

		if (track[route->primary[i].node].type == NODE_BRANCH) {
			if (dist > min_dist) {
				if (route->primary[i].edge == DIR_STRAIGHT) {
					if(route->primary[i].node == switch_to_set) {
						SetSwitch(track[route->primary[i].node].num, SWITCH_STRAIGHT);
					}
				} else if (route->primary[i].edge == DIR_CURVED) {
					if(route->primary[i].node == switch_to_set) {
						SetSwitch(track[route->primary[i].node].num, SWITCH_CURVED);
					}
				}
			}
			//Set switches for secondary route
			secondary_index = route->primary[i].secondary_route_index;
			//Start at 1, cuz the first secondary route node is the current node! We really don't want
			// to set it to the secondary setting!
			for (k = 1; k < route->secondary_size[secondary_index]; k++) {
				if (route->secondary[secondary_index][k].length_reserved <= 0 || (k
						< route->secondary_size[secondary_index]
						&& route->secondary[secondary_index][k].length_reserved <= 0)) {
					//Stop setting switches when we run out of route
					break;
				}
				if (track[route->secondary[secondary_index][k].node].type == NODE_BRANCH) {
					if (route->secondary[secondary_index][k].edge == DIR_STRAIGHT) {
						if(route->secondary[secondary_index][k].node == switch_to_set) {
							SetSwitch(track[route->secondary[secondary_index][k].node].num, SWITCH_STRAIGHT);
						}
					} else if (route->secondary[secondary_index][k].edge == DIR_CURVED) {
						if(route->secondary[secondary_index][k].node == switch_to_set) {
							SetSwitch(track[route->secondary[secondary_index][k].node].num, SWITCH_CURVED);
						}
					}
				}
			}
		} else if(track[route->primary[i].node].type == NODE_MERGE) {
			if (dist > min_dist && i > 0) {
				//Set the reverse switch, just to be sure
				if(track[route->primary[i].node].reverse->edge[0].dest->reverse->index == route->primary[i - 1].node) {
					if(route->primary[i].node == switch_to_set) {
						SetSwitch(track[route->primary[i].node].reverse->num, SWITCH_STRAIGHT);
					}
				} else if(track[route->primary[i].node].reverse->edge[1].dest->reverse->index == route->primary[i - 1].node) {
					if(route->primary[i].node == switch_to_set) {
						SetSwitch(track[route->primary[i].node].reverse->num, SWITCH_CURVED);
					}
				}
			}
		}
		if (route->primary[i].reverse_here) {
			//Don't set switches beyond a reverse (headache)
			return;
		}
		//Advance distance count
		dist += track[route->primary[i].node].edge[route->primary[i].edge].dist;
	}

}

/*
 * Sensor Prediction Functions
 */

//Distance until the next sensor we expect to hit from the current position
//TESTED
int next_primary_sensor_distance(struct track_node *track, struct position * pos, struct train_route * route,
		int * sensor) {
	return next_sensor_distance(track, pos, route->primary, route->primary_size, sensor);
}

//Find length (in mm) of secondary route
//TESTED
int first_secondary_sensor_distance(struct track_node * track, struct train_route *route, int primary_index,
		int * sensor) {
	int secondary_route_index = route->primary[primary_index].secondary_route_index;
	if (secondary_route_index < 0) {
		return INT_MAX;
	}

	struct route_node * secondary = route->secondary[secondary_route_index];
	int secondary_size = route->secondary_size[secondary_route_index];
	int dist = 0, i = 0;

	for (i = 0; i < secondary_size; i++) {
		if (track[secondary[i].node].type == NODE_SENSOR) {
			*sensor = secondary[i].node;
			return dist;
		}
		if (secondary[i].reverse_here) {
			//We shouldn't be reversing on a secondary route... I would hope
			assert(0, "secondary_route_length: reversing on secondary route");
		}
		if (secondary[i].length_reserved < track[secondary[i].node].edge[secondary[i].edge].dist) {
			dist += secondary[i].length_reserved;
			return dist;
		}
		dist += track[secondary[i].node].edge[secondary[i].edge].dist;
	}
	return INT_MAX;
}

//Make a prediction for which sensors we could hit next, returns the number of sensors
int predict_sensors_classical(struct track_node *track, struct position * pos, struct train_route * route, int * sensors,
		int * distances, int * primary) {
	int pos_index = 0, i = 0, next_sensor_index = 0, sensor_index = 0;
	int num_sensors = 0;
	struct position curpos, sensorpos;

	//Find the route index of the given position
	for (pos_index = 0; pos_index < route->primary_size && route->primary[pos_index].node != pos->node; pos_index++)
		;
	if (pos_index == route->primary_size) {
		//We're not on the route!
		num_sensors = 0;
		return 0;
	}

	curpos.node = pos->node;
	curpos.offset = pos->offset;

	//Find primary sensor, that we expect to run into first
	distances[num_sensors] = next_sensor_distance(track, &curpos, route->primary, route->primary_size, sensors
			+ num_sensors);
	primary[num_sensors] = TRUE;
	//If we went off the route looking for the secondary sensor, ignore the result we got, but normally we continue
	if (distances[num_sensors] < INT_MAX && sensors[num_sensors] > -1) {
		sensorpos.node = sensors[num_sensors];
		sensorpos.offset = 0;
		distances[num_sensors] = primary_route_distance(route, pos, &sensorpos);
		primary[num_sensors] = TRUE;
		num_sensors++;

		//Find the route index of the first sensor
		for (next_sensor_index = 0; next_sensor_index < route->primary_size && route->primary[next_sensor_index].node
				!= sensors[0]; next_sensor_index++)
			;
		if (next_sensor_index >= route->primary_size) {
			//We're not on the route!
			num_sensors = 0;
			return 0;
		}

		//If we aren't off the route now, look for the sensor after the primary sensor (in case the primary sensor
		//fails
		if (next_sensor_index < route->primary_size) {
			curpos.offset = 1;
			curpos.node = sensors[0];
			distances[num_sensors] = next_sensor_distance(track, &curpos, route->primary, route->primary_size, sensors
					+ num_sensors);
			primary[num_sensors] = TRUE;
			//If we went off the route looking for the secondary sensor, ignore the result we got
			if (distances[num_sensors] < INT_MAX && sensors[num_sensors] >= 0) {
				sensorpos.node = sensors[num_sensors];
				sensorpos.offset = 0;
				distances[num_sensors] = primary_route_distance(route, pos, &sensorpos);
				num_sensors++;
			}
		}
	}

	//Traverse the primary route, look for the first sensors along any secondary routes before we reach the
	//primary sensor
	for (i = pos_index; i < route->primary_size && route->primary[i].node != sensors[0]; i++) {
		//CommandOutput("[predict_sensors] Looking for branch at %s", track[route->primary[i].node].name);
		if (track[route->primary[i].node].type == NODE_BRANCH) {
			//CommandOutput("[predict_sensors] Looking for sensor along secondary route starting at %s", track[route->primary[i].node].name);
			//Look along the secondary route
			distances[num_sensors] = first_secondary_sensor_distance(track, route, i, sensors + num_sensors);
			//CommandOutput("[predict_sensors] %d %d", distances[num_sensors], sensors[num_sensors]);
			//If we went off the route looking for the secondary sensor, ignore the result we got
			if (distances[num_sensors] < INT_MAX && sensors[num_sensors] >= 0) {
				sensor_index = secondary_route_index(route, route->primary[i].secondary_route_index, 0,
						sensors[num_sensors]);
				sensorpos.node = sensors[num_sensors];
				sensorpos.offset = 0;
				distances[num_sensors]
						= route->secondary[route->primary[i].secondary_route_index][sensor_index].distance
								- (route->primary[primary_route_index(route, 0, pos->node)].distance + pos->offset);
				primary[num_sensors] = FALSE;
				num_sensors++;
			}
			//Delay(100);
		}
	}
	return num_sensors;
}

//TODO: More testing for quantum train state
int predict_sensors_quantum(struct track_node *track, struct position * pos, struct train_route * route, int * sensors, int * distances, int * primary, int last_sensor) {
	struct position pos2;
	int i;
	int num_sensors_init, num_sensors;
	int dist_from_last_sensor;

	if(last_sensor < 0 || primary_route_index(route, 0, last_sensor) < 0) {
		last_sensor = route->primary[0].node;
	}

	pos2.node = last_sensor;
	pos2.offset = 0;
	dist_from_last_sensor = primary_route_distance(route,&pos2, pos);
	num_sensors_init = predict_sensors_classical(track, &pos2, route, sensors, distances, primary);
	num_sensors = 0;
	for(i = 0; i< num_sensors_init; i++) {
		if(distances[i] > dist_from_last_sensor && distances[i] < 900000) {
			sensors[num_sensors] = sensors[i];
			distances[num_sensors] = distances[i] - dist_from_last_sensor;
			primary[num_sensors] = primary[i];
			num_sensors++;
		}
	}
	return num_sensors;
}


//Helper function, to determine the distance until the next sensor we expect to hit from the current position
//from the given route section
int next_sensor_distance(struct track_node *track, struct position * pos, struct route_node * route_nodes,
		int route_length, int * sensor) {
	int dist = 0, i = 0;
	//Set i to the position of the current node on the route
	for (i = 0; i < route_length && route_nodes[i].node != pos->node; i++)
		;
	//Start looking at the node after the current one
	if (i >= route_length) {
		*sensor = -1;
		return INT_MAX;
	}

	if (route_nodes[i].edge == 0) {
		//Route follows edge 0
		dist = track[route_nodes[i].node].edge[0].dist - pos->offset;
	} else if (track[route_nodes[i].node].type == NODE_BRANCH && route_nodes[i].edge == 1) {
		//Route follows edge 1
		dist = track[route_nodes[i].node].edge[1].dist - pos->offset;
	} else if (route_nodes[i].reverse_here) {
		//TODO: Sensible value when we want to reverse
		return INT_MAX;
	}
	i++;

	//While the offset from the current node is greater than the next edge length, we go forward looking
	//for a new sensor
	while (i < route_length) {
		if (route_nodes[i].edge == 0) {
			//Route follows edge 0
			dist += track[route_nodes[i].node].edge[0].dist;
		} else if (track[route_nodes[i].node].type == NODE_BRANCH && route_nodes[i].edge == 1) {
			//Route follows edge 1
			dist += track[route_nodes[i].node].edge[1].dist;
		} else if (route_nodes[i].reverse_here) {
			//We reverse before the next sensor
			*sensor = -1;
			return INT_MAX;
		}
		if (track[route_nodes[i].node].type == NODE_SENSOR) {
			*sensor = route_nodes[i].node;
			return dist;
		}
		i++;
	}
	*sensor = -1;
	return INT_MAX; //We'll never hit another sensor again - DOOM I SAY!
}

/*
 * Stop distance functions
 */

//Find length (in mm) of secondary route
//TESTED
int secondary_route_length(struct track_node * track, struct train_route *route, int primary_index) {
	int secondary_route_index = route->primary[primary_index].secondary_route_index;
	if (secondary_route_index < 0) {
		return 0;
	}

	struct route_node * secondary = route->secondary[secondary_route_index];
	int secondary_size = route->secondary_size[secondary_route_index];
	int dist = 0, i = 0;

	for (i = 0; i < secondary_size; i++) {
		if (secondary[i].reverse_here) {
			//We shouldn't be reversing on a secondary route... I would hope
			assert(0, "secondary_route_length: reversing on secondary route");
		}
		if (secondary[i].length_reserved < track[secondary[i].node].edge[secondary[i].edge].dist) {
			//If this node is a merge, we may have problems... we need to retroactively stop about
			//200mm before the merge (if we have space to do so)
			if(secondary[i].length_reserved == 0 && track[secondary[i].node].type == NODE_MERGE) {
				dist = MAX(dist - CONFIG_TRAIN_SAFE_MERGE_STOP_DISTANCE,0);
			} else {
				dist += secondary[i].length_reserved;
			}
			return dist;
		}
		dist += track[secondary[i].node].edge[secondary[i].edge].dist;
	}
	return dist;
}

//Find guaranteed length (in mm) we can travel along the route
//TESTED
int route_guaranteed_length(struct track_node * track, struct position *pos, struct train_route * route,
		int last_sensor) {
	int i = 0;
	int dist = 0, min_stop_dist = INT_MAX;
	int revlength = 0;
	int secondary_route_dist = 0;
	struct position last_sensor_pos;
	int dist_from_last_sensor;
	//Quantum train!
	//We could be in a situation where we think we've passed a branch, but haven't hit a
	//sensor yet, and are really on the secondary route. We need to set speeds, etc. based
	//on this situation


	if (last_sensor >= 0) {
		i = primary_route_index(route, 0, last_sensor);
		if (i < 0) {
			i = 0;
		}
	} else {
		i = 0;
	}

	last_sensor_pos.node = route->primary[i].node;
	last_sensor_pos.offset = 0;
	dist_from_last_sensor = primary_route_distance(route, &last_sensor_pos, pos);

	//setup dist variable
	if (route->primary[i].edge == 0) {
		//Route follows edge 0
		dist = track[route->primary[i].node].edge[0].dist - pos->offset;
	} else if (track[route->primary[i].node].type == NODE_BRANCH && route->primary[i].edge == 1) {
		//Route follows edge 1
		dist = track[route->primary[i].node].edge[1].dist - pos->offset;
	} else if (route->primary[i].reverse_here) {
		return MIN(route->primary[i].length_reserved,CONFIG_TRAIN_SAFE_REVERSE_BUFFER)- pos->offset;
	}

	//Check along quantum train routes
	while (i < route->primary_size && route->primary[i].node != pos->node) {
		if (route->primary[i].reverse_here) {
			//We reversed somewhere, reset the minimum stop distance and keep going till we hit the right node
			min_stop_dist = INT_MAX;
		} else if (track[route->primary[i].node].type == NODE_BRANCH) {
			//This node is a branch. We're limited by the length of reservation along the secondary route
			//ignore if we're reasonably sure we would have triggered a sensor if we were along the
			//secondary route
			//TODO: a little more testing for this... but looks good now
			secondary_route_dist = secondary_route_length(track, route, i) + dist - dist_from_last_sensor;
			int secondary_sensor_dist, sensor;
			secondary_sensor_dist = first_secondary_sensor_distance(track, route, i, &sensor) + dist
					- dist_from_last_sensor;
			if (secondary_route_dist > 0 && secondary_sensor_dist > 0 && min_stop_dist > secondary_route_dist) {
				min_stop_dist = secondary_route_dist;
			}
		}
		dist += track[route->primary[i].node].edge[route->primary[i].edge].dist;
		i++;
	}

	//i should be set to the current position's node now

	if (i == route->primary_size) {
		return 0;
		assert(0, "stop_offset: pos not on route!");
	}
	dist = -pos->offset;

	while (i < route->primary_size) {
		if (route->primary[i].reverse_here) {
			//TODO: sensible stop distance here for reversing
			//Reverse node coming up, we need to stop before it
			//If we have a super-long track section, we don't need to use the whole thing
			revlength = MIN(route->primary[i].length_reserved,CONFIG_TRAIN_SAFE_REVERSE_BUFFER);
			if (min_stop_dist > dist + revlength) {
				min_stop_dist = dist + revlength;
			}
		} else if (route->primary[i].length_reserved < track[route->primary[i].node].edge[route->primary[i].edge].dist) {
			//We don't have this section of track completely reversed
			if (min_stop_dist > dist + route->primary[i].length_reserved) {
				min_stop_dist = dist + route->primary[i].length_reserved;
			}
			//If this node is a merge, we may have problems... we need to retroactively stop about
			//200mm before the merge (if we have space to do so)
			if(route->primary[i].length_reserved == 0 && track[route->primary[i].node].type == NODE_MERGE) {
				min_stop_dist = MAX(dist - CONFIG_TRAIN_SAFE_MERGE_STOP_DISTANCE,0);
			}
/*
		} else if (route->destination.node != -1 && route->primary[i].node == route->destination.node) {
			//This node is our destination
			if (min_stop_dist > dist + route->destination.offset) {
				min_stop_dist = dist + route->destination.offset;
			}
*/
		} else if (track[route->primary[i].node].type == NODE_EXIT) {
			//This node is an exit
			//No exit
			if (min_stop_dist > dist) {
				min_stop_dist = dist;
			}
		} else if (track[route->primary[i].node].type == NODE_BRANCH) {
			//This node is a branch. We're limited by the length of reservation along the secondary route
			secondary_route_dist = secondary_route_length(track, route, i) + dist;
			if (min_stop_dist > secondary_route_dist) {
				min_stop_dist = secondary_route_dist;
			}
		}
		//Check if we have to stop on the current section
		if (min_stop_dist <= dist) {
			return min_stop_dist;
		}
		dist += track[route->primary[i].node].edge[route->primary[i].edge].dist;
		i++;
	}
	return dist;
}

//Find the offset into the current track section which we'll need to stop the train at
int stop_offset(struct track_node * track, struct position *pos, struct train_route * route, int stop_distance,
		int last_sensor) {
	int sensor = 0;
	int min_stop_dist = route_guaranteed_length(track, pos, route, last_sensor);
	int next_sensor_dist = next_primary_sensor_distance(track, pos, route, &sensor);

	//Check if we have to stop on the current section
	if (min_stop_dist < INT_MAX) {
		if (min_stop_dist <= next_sensor_dist + stop_distance) {
			return min_stop_dist - stop_distance;
		}
		return INT_MAX;
	}
	return INT_MAX;
}

