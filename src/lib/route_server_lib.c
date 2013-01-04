/*
 * route_server_lib.c
 */

#include <userspace.h>

int get_track_number() {
        int tid;
        struct number_message init_msg;
        //Get a message from the CLI indicating which track is in use
        Receive(&tid, (char*) &init_msg, sizeof(init_msg));
        Reply(tid, (char*) 0, 0);
        assert(init_msg.message_type == ROUTE_SERVER_INIT_MESSAGE,"init_msg.message_type == ROUTE_SERVER_INIT_MESSAGE");
        return init_msg.num;
}

//Notify the train that it's obtained a reservation
void notify_reservation(struct route_server_data *DATA, int train_index, int node, int edge, int length_reserved) {
        struct train_reservation_update_message msg;
        msg.message_type = TRAIN_ROUTE_RESERVATION_UPDATE_MESSAGE;
        msg.node = node;
        msg.edge = edge;
        msg.length_reserved = length_reserved;
        //CommandOutput("Gave reservation to %d at %s(%d) for %d",DATA->train_data[train_index].train_number, DATA->track[node].name, edge, length_reserved);
        CourierSend(DATA->train_data[train_index].tid, (char *) &msg, sizeof(msg));
}


/*
 * Start copy here
 */

void init_route_server_data(struct route_server_data *DATA) {
	int i, j, k;

	for (i = 0; i < CONFIG_MAX_TRAINS; i++) {
		DATA->train_data[i].train_number = -1;
		DATA->train_data[i].train_index = -1;
		DATA->train_data[i].train_length = 0;
		DATA->train_data[i].requested_route_length = 0;
		DATA->train_data[i].last_pos.node = -1;
		DATA->train_data[i].last_pos.offset = 0;
		DATA->train_data[i].tid = -1;
		init_route(&(DATA->train_data[i].route));
	}

	DATA->num_trains = 0;
	DATA->reservations.num_trains = 0;

	for (i = 0; i < TRACK_MAX; i++) {
		for (j = 0; j < 2; j++) {
			for (k = 0; k < CONFIG_MAX_TRAINS; k++) {
				DATA->reservations.status[i][j][k] = RSV_NONE;
			}
		}
	}
	DATA->track_number = get_track_number();
	if (DATA->track_number == TRACK_A) {
		init_tracka(DATA->track);
	} else if (DATA->track_number == TRACK_B) {
		init_trackb(DATA->track);
	}
}

//Get the index for a given train number
int get_train_index(struct route_server_data *DATA, int train_number) {
	int i;
	int train_index = -1;
	for (i = 0; i < CONFIG_MAX_TRAINS; i++) {
		if (DATA->train_data[i].train_number == train_number) {
			train_index = i;
			break;
		} else if (DATA->train_data[i].train_number <= 0) {
			DATA->train_data[i].train_number = train_number;
			train_index = i;
			break;
		}
	}
	assert(train_index >= 0, "train_index >= 0");

	if (DATA->num_trains < train_index + 1) {
		DATA->num_trains = train_index + 1;
	}
	if (DATA->reservations.num_trains < train_index + 1) {
		DATA->reservations.num_trains = train_index + 1;
	}

	DATA->train_data[train_index].train_index = train_index;

	return train_index;
}

void new_route(struct route_server_data *DATA, int train_number, struct position *pos1, struct position *pos2) {
	int train_index = get_train_index(DATA, train_number);
	if (DATA->train_data[train_index].route.primary_size > 0) {
		//clear_reservations(DATA, train_index);
	}
	int length = shortest_route(DATA, train_index, &(DATA->train_data[train_index].route), pos1, pos2, CONFIG_REVERSE_DISTANCE_COST, CONFIG_RESERVED_EDGE_COST);
	//CommandOutput("Shortest Route Length: %d", length);
	//Delay(300);

	//DATA->train_data[train_index].route.destination.node = pos2->node;
	//DATA->train_data[train_index].route.destination.offset = pos2->offset;
}

/*
 * Route creation functions
 */

//Add a secondary route at the given node, returns length in number of nodes
//TESTED
int add_secondary_route(struct track_node *track, int primary_index, int expected_direction,
		struct train_route * route, int desired_length) {
	int curnode = route->primary[primary_index].node;
	int secondary_direction = 0;
	struct route_node * secondary = route->secondary[route->num_secondary_routes];
	int swstatus = 0;
	int firstsensordist = INT_MAX;
	int i;
	int nextnode, nextdist;

	if (expected_direction == DIR_CURVED) {
		secondary_direction = DIR_STRAIGHT;
	} else if (expected_direction == DIR_STRAIGHT) {
		secondary_direction = DIR_CURVED;
	}

	//Set up first node along secondary route
	secondary[0].node = curnode;
	secondary[0].distance = route->primary[primary_index].distance;
	secondary[0].secondary_route_index = -1;
	secondary[0].length_reserved = 0;
	secondary[0].edge = secondary_direction;

	//Point at the next node along secondary route
	nextnode = track[curnode].edge[secondary_direction].dest->index;
	nextdist = route->primary[primary_index].distance + track[curnode].edge[secondary_direction].dist;

	//Loop through a reasonable secondary route, adding nodes until we go as far as we need
	for (i = 1; (i < CONFIG_MAX_SECONDARY_ROUTE_SIZE) && (secondary[i - 1].distance - firstsensordist < desired_length); i++) {
		curnode = nextnode;
		secondary[i].node = nextnode;
		secondary[i].distance = nextdist;
		secondary[i].secondary_route_index = -1;
		if (track[curnode].type == NODE_SENSOR && firstsensordist == INT_MAX) {
			firstsensordist = secondary[i].distance;
		}
		if (track[curnode].type == NODE_BRANCH) {
			//handle branch
			//TODO: Select a nice route along the branch, rather than relying on the current status
			swstatus = SwitchStatus(track[curnode].num);
			if (swstatus == SWITCH_STRAIGHT) {
				nextdist = track[curnode].edge[DIR_STRAIGHT].dist + secondary[i].distance;
				nextnode = track[curnode].edge[DIR_STRAIGHT].dest->index;
				secondary[i].length_reserved = 0;
				secondary[i].edge = DIR_STRAIGHT;
			} else if (swstatus == SWITCH_CURVED) {
				nextdist = track[curnode].edge[DIR_CURVED].dist + secondary[i].distance;
				nextnode = track[curnode].edge[DIR_CURVED].dest->index;
				secondary[i].edge = DIR_CURVED;
				secondary[i].length_reserved = 0;
			} else {
				secondary[i].edge = 0;
				secondary[i].length_reserved = 0;
				break;
			}
		} else if (track[curnode].type == NODE_EXIT) {
			secondary[i].edge = 0;
			secondary[i].length_reserved = 0;
			break;
		} else {
			//normal case: only one path
			nextdist = track[curnode].edge[0].dist + secondary[i].distance;
			nextnode = track[curnode].edge[0].dest->index;
			secondary[i].edge = DIR_AHEAD;
			secondary[i].length_reserved = 0;
		}
	}

	//Add the secondary route to the route structure
	route->secondary_size[route->num_secondary_routes] = i;
	route->primary[primary_index].secondary_route_index = route->num_secondary_routes;
	route->num_secondary_routes++;
	return i;
}

int can_reverse_here(struct track_node *track, int node, int edge) {
	int length = track[node].edge[edge].dist;

	if (length > CONFIG_TRAIN_SAFE_REVERSE_BUFFER) {
		return 1;
	}
	int curnode = track[node].reverse->index;

	while (track[curnode].type != NODE_EXIT && track[curnode].type != NODE_BRANCH) {
		length += track[curnode].edge[0].dist;
		curnode = track[curnode].edge[0].dest->index;
	}
	if (length > CONFIG_TRAIN_SAFE_REVERSE_BUFFER) {
		return 1;
	}
	return 0;
}

//Find the shortest route from node1 to node2. Returns it's length
//TESTED
int shortest_route(struct route_server_data * DATA, int train_index, struct train_route * route,
		struct position * pos1, struct position * pos2, int reverse_cost, int reserved_cost) {
	struct track_node * track = DATA->track;

	//Arrays for first part of algorithm
	struct position fwd_end_pos, rev_end_pos, begin_pos, pos2_rev;
	pos2_rev = reverse_position(track, pos2);

	//We route from the current position of the pickup to the position where we want the front of the  train to go
	fwd_end_pos = train_front(track, pos2, CONFIG_TRAIN_MAX_LENGTH_FROM_PICKUP);
	rev_end_pos = train_front(track, &pos2_rev, CONFIG_TRAIN_MAX_LENGTH_FROM_PICKUP);

	int node1 = pos1->node, node2 = -1;
	int fwd_node2 = fwd_end_pos.node;
	int rev_node2 = rev_end_pos.node;
	int fwd_end_processed = 0, rev_end_processed = 0;

	int distance[TRACK_MAX * 2];
	int cost[TRACK_MAX * 2];
	short parent[TRACK_MAX * 2];
	short parentedge[TRACK_MAX * 2];
	short parent_reverse_here[TRACK_MAX * 2];

	int curnode = 0, curcost = 0, curedge = 0, cur_reverse_here, nextindex = 0, curtracknode, cur_parent_reverse_here;
	int i = 0;
	int pathdistance = 0, pathcost = 0;
	int routelength = 0;
	int routedistance = 0;
	//int reverse_cost = CONFIG_REVERSE_DISTANCE_COST;

	struct heap node_heap;
	int node_heap_elements[TRACK_MAX], node_heap_priorities[TRACK_MAX * 2];
	heap_init(&node_heap, node_heap_elements, node_heap_priorities, TRACK_MAX * 2);

	heap_add(&node_heap, node1, 0);

	curnode = -1;

	//Initialize the information array
	for (i = 0; i < TRACK_MAX * 2; i++) {
		distance[i] = INT_MAX;
		cost[i] = INT_MAX;
		parent[i] = -1;
		parentedge[i] = -1;
		parent_reverse_here[i] = 0;
	}

	//Start off at node1, which has no parent, and with no reverse in the parent
	distance[node1] = 0;
	cost[node1] = 0;
	parent[node1] = -1;
	parentedge[node1] = -1;
	parent_reverse_here[node1] = 0;
	distance[node1 + TRACK_MAX] = 0;
	cost[node1 + TRACK_MAX] = 0;
	parent[node1 + TRACK_MAX] = -1;
	parentedge[node1 + TRACK_MAX] = -1;
	parent_reverse_here[node1 + TRACK_MAX] = 0;

	while (heap_min_priority(&node_heap) >= 0 && heap_min_priority < INT_MAX) {
		//Get the next closest node to the origin from the top of the heap
		curcost = heap_min_priority(&node_heap);
		curnode = heap_remove_min(&node_heap);
		cur_parent_reverse_here = curnode / TRACK_MAX;
		curtracknode = curnode % TRACK_MAX;
		if (cost[curnode] < curcost) {
			//We've found a shorter path to the node, and should have already processed it
			//Ignore it!
		} else {
			//CommandOutput( "Distance to node %s is %d", track[curnode].name, curdist );
			//Check if we're processing any of the destination nodes
			//We have no shorter path than the one we know now, let's update other vertices based on this
			if (curnode == fwd_node2) {
				fwd_end_processed = 1;
			}
			if (curnode == rev_node2) {
				rev_end_processed = 1;
			}
			if (rev_end_processed || fwd_end_processed) {
				break;
			}
			//Update along the forward direction (edge 0), except if this is an exit node
			if (track[curtracknode].type != NODE_EXIT && (!cur_parent_reverse_here || track[curtracknode].type
					!= NODE_BRANCH || parent[curnode] == track[curtracknode].edge[0].dest->reverse->index)) {
				pathdistance = distance[curnode] + track[curtracknode].edge[0].dist;
				pathcost = cost[curnode] + track[curtracknode].edge[0].dist;
				nextindex = track[curtracknode].edge[0].dest->index;
				if(reserve_conflict(DATA,train_index, curnode % TRACK_MAX, 0)) {
					pathcost += reserved_cost;
				}
				if(pathcost < 0) {
					pathcost = INT_MAX;
				}
				if (cost[nextindex] > pathcost) {
					//Shorter path found to a node: let's update it
					distance[nextindex] = pathdistance;
					cost[nextindex] = pathcost;
					parent[nextindex] = curnode;
					parentedge[nextindex] = 0;
					//parent_reverse_here[nextindex] = 0;
					heap_add(&node_heap, nextindex, cost[nextindex]);
				}
				if (!cur_parent_reverse_here && can_reverse_here(track, curtracknode, 0)) {
					//Reverse along this edge
					nextindex = track[curtracknode].edge[0].dest->reverse->index + TRACK_MAX;
					pathdistance = distance[curnode] + track[curtracknode].edge[0].dist;
					pathcost = cost[curnode] + reverse_cost;
					if(reserve_conflict(DATA,train_index, curnode % TRACK_MAX, 0)) {
						pathcost += reserved_cost;
					}
					if(pathcost < 0) {
						pathcost = INT_MAX;
					}
					if (cost[nextindex] > pathcost) {
						distance[nextindex] = pathdistance;
						cost[nextindex] = pathcost;
						parent[nextindex] = curnode;
						parentedge[nextindex] = 0;
						//parent_reverse_here[nextindex] = 1;
						heap_add(&node_heap, nextindex, cost[nextindex]);
					}
				}
			}
			//If the node is a branch, we can also update along the curved direction
			if (track[curtracknode].type == NODE_BRANCH && (!cur_parent_reverse_here || parent[curnode]
					== track[curtracknode].edge[1].dest->reverse->index)) {
				pathdistance = distance[curnode] + track[curtracknode].edge[1].dist;
				pathcost = cost[curnode] + track[curtracknode].edge[1].dist;
				nextindex = track[curtracknode].edge[1].dest->index;
				if(reserve_conflict(DATA,train_index, curnode % TRACK_MAX, 1)) {
					pathcost += reserved_cost;
				}
				if(pathcost < 0) {
					pathcost = INT_MAX;
				}
				if (cost[nextindex] > pathcost) {
					//Shorter path found to a node: let's update it
					distance[nextindex] = pathdistance;
					cost[nextindex] = pathcost;
					parent[nextindex] = curnode;
					parentedge[nextindex] = 1;
					//parent_reverse_here[nextindex] = 0;
					heap_add(&node_heap, nextindex, cost[nextindex]);
				}
				if (!cur_parent_reverse_here && can_reverse_here(track, curtracknode, 1)) {
					//reverse along this edge
					nextindex = track[curtracknode].edge[1].dest->reverse->index + TRACK_MAX;
					pathdistance = distance[curnode] + track[curtracknode].edge[1].dist;
					pathcost = cost[curnode] + reverse_cost;
					if(reserve_conflict(DATA,train_index, curnode % TRACK_MAX, 1)) {
						pathcost += reserved_cost;
					}
					if(pathcost < 0) {
						pathcost = INT_MAX;
					}
					if (cost[nextindex] > pathcost) {
						distance[nextindex] = pathdistance;
						cost[nextindex] = pathcost;
						parent[nextindex] = curnode;
						parentedge[nextindex] = 1;
						//parent_reverse_here[nextindex] = 1;
						heap_add(&node_heap, nextindex, cost[nextindex]);
					}
				}
			}

			//We can also reverse! By reversing we mean: position the train so it is on the current
			// edge, and reverse while stationary. Thus, after the reverse we end up on the
			// edge/direction represented by the reverse of the node following this one. Then, the
			// next node we pass over is the reverse of the current node.

			//TODO: Fix reversing so we always go past switches. Should be fixed in here so
			// that rear distance to the next switch > train length + buffer
			//Should be fixed in path finding so other components don't need to worry about it

		}
	}

/*
	for(i = 0; i < 2 * TRACK_MAX; i++) {
		printf("%s:%d:%s:%d:%d\n",track[i % TRACK_MAX].name, cost[i], track[parent[i] % TRACK_MAX].name, parentedge[i], i / TRACK_MAX);
	}
*/

	//Pick whether we want to go to the node or its reverse
	if (cost[fwd_node2] < cost[rev_node2]) {
		node2 = fwd_node2;
		route->destination = *pos2;
	} else {
		node2 = rev_node2;
		route->destination = pos2_rev;
	}

	if (cost[node2] == INT_MAX) {
		//Can't find the destination
		return INT_MAX;
	}

	//Retrieve the route from the parent array in reverse order

	short backwardsroute[TRACK_MAX];
	short backwardsedge[TRACK_MAX];

	for (i = 0; i < TRACK_MAX; i++) {
		backwardsroute[i] = -1;
		backwardsedge[i] = 0;
	}

	//The last node in the path will be node2 (with an unspecified direction after it)
	if (cost[node2] <= cost[node2 + TRACK_MAX]) {
		curnode = node2;
	} else {
		curnode = node2 + TRACK_MAX;
	}

	curedge = 0;
	routelength = 0;
	while (curnode != node1 && curnode != -1) {
		backwardsroute[routelength] = curnode;
		backwardsedge[routelength] = curedge;
		routelength++;
		curedge = parentedge[curnode];
		curnode = parent[curnode];
	}

	if (curnode == -1) {
		//We've gotten really lost...
		return INT_MAX;
	}
	//Finish off the route (curnode should be node1)
	backwardsroute[routelength] = curnode;
	backwardsedge[routelength] = curedge;
	routelength++;

	//Create the route struct by reversing the order and properly initialising everything
	route->num_secondary_routes = 0;
	route->primary_size = 0;
	//Loop through the entire reverse route
	for (i = 0; i < routelength; i++) {
		route->primary[i].node = backwardsroute[routelength - i - 1] % TRACK_MAX;
		curnode = route->primary[i].node;
		route->primary[i].distance = distance[backwardsroute[routelength - i - 1]];
		route->primary[i].edge = backwardsedge[routelength - i - 1];
		route->primary[i].length_reserved = 0;
		route->primary[i].secondary_route_index = -1;
		if (i < routelength - 1) {
			route->primary[i].reverse_here = backwardsroute[routelength - i - 2] / TRACK_MAX;
		} else {
			route->primary[i].reverse_here = 0;
		}
	}

	route->primary_size = i;
	route->primary[route->primary_size].node = -1;
	route->primary[route->primary_size].distance = INT_MAX;

	//Find the total distance we go along the path
	routedistance = route->primary[route->primary_size - 1].distance;
	if (track[route->primary[route->primary_size - 1].node].type != NODE_EXIT) {
		routedistance += track[route->primary[route->primary_size - 1].node].edge[route->primary[route->primary_size
				- 1].edge].dist;
	}

	int secondary_route_length[TRACK_MAX];
	for (i = 0; i < TRACK_MAX; i++) {
		secondary_route_length[i] = -1;
	}

	//Look through the path and add secondary routes
	for (i = routelength - 1; i >= 0; i--) {
		if (i < routelength && track[route->primary[i].node].type == NODE_BRANCH) {
			//We need to add a secondary route here
			if (route->primary[i].reverse_here == 1) {
				//Update the distance, since we'll always stop by the end of this reverse node
				routedistance = route->primary[i].distance
						+ track[route->primary[i].node].edge[route->primary[i].edge].dist;
			}
			//We only want a secondary route that goes as far as the primary route goes before stopping
			secondary_route_length[i] = MIN(CONFIG_SECONDARY_ROUTE_LENGTH, routedistance - route->primary[i].distance);
		} else {
			secondary_route_length[i] = -1;
		}
	}

	for (i = 0; i < routelength; i++) {
		if (secondary_route_length[i] > 0) {
			//We only want a secondary route that goes as far as the primary route goes before stopping
			add_secondary_route(track, i, route->primary[i].edge, route, secondary_route_length[i]);
		}
	}

	return cost[node2];
}

/*
 * Reservation functions
 */

int can_reserve(struct route_server_data * DATA, int train_index, int node, int edge) {
	struct track_node * track = DATA->track;
	struct reservation_data * reservations = &(DATA->reservations);
	int i = 0;
	int reverse_node = 0;
	int reverse_edge = 0;

	if (track[node].type != NODE_EXIT) {
		reverse_node = track[node].edge[edge].dest->reverse->index;
		if (track[reverse_node].edge[0].dest->index == track[node].reverse->index) {
			reverse_edge = 0;
		} else if (track[reverse_node].edge[1].dest->index == track[node].reverse->index) {
			reverse_edge = 1;
		} else {
			assert(0, "Failure in can_reserve");
		}
	}

	for (i = 0; i < reservations->num_trains; i++) {
		if (i != train_index) {
			if (reservations->status[node][edge][i] == RSV_HELD) {
				return FALSE;
			}

			if (track[node].type != NODE_EXIT && reservations->status[reverse_node][reverse_edge][i] == RSV_HELD) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

int reserve_conflict(struct route_server_data * DATA, int train_index, int node, int edge) {
	struct track_node * track = DATA->track;
	struct reservation_data * reservations = &(DATA->reservations);
	int i = 0;
	int reverse_node = 0;
	int reverse_edge = 0;
	int other_edge = 1-edge;

	if (track[node].type != NODE_EXIT) {
		reverse_node = track[node].edge[edge].dest->reverse->index;
		if (track[reverse_node].edge[0].dest->index == track[node].reverse->index) {
			reverse_edge = 0;
		} else if (track[reverse_node].edge[1].dest->index == track[node].reverse->index) {
			reverse_edge = 1;
		} else {
			assert(0, "Failure in can_reserve");
		}
	}

	for (i = 0; i < reservations->num_trains; i++) {
		if (i != train_index) {
			if (reservations->status[node][edge][i] == RSV_HELD) {
				//CommandOutput("reserve_conflict: %s(%d) %d %d", DATA->track[node].name, edge, i, train_index);
				return TRUE;

			}

			if (track[node].type != NODE_EXIT && reservations->status[reverse_node][reverse_edge][i] == RSV_HELD) {
				//CommandOutput("reserve_conflict: %s(%d) %d %d", DATA->track[node].name, edge, i, train_index);
				return TRUE;
			}
		}
	}
	if (track[node].type == NODE_BRANCH) {
		reverse_node = track[node].edge[other_edge].dest->reverse->index;
		if (track[reverse_node].edge[0].dest->index == track[node].reverse->index) {
			reverse_edge = 0;
		} else if (track[reverse_node].edge[1].dest->index == track[node].reverse->index) {
			reverse_edge = 1;
		} else {
			assert(0, "Failure in can_reserve");
		}
		for (i = 0; i < reservations->num_trains; i++) {
			if (i != train_index) {
				if (reservations->status[node][other_edge][i] == RSV_HELD) {
					//CommandOutput("reserve_conflict: %s(%d) %d %d", DATA->track[node].name, edge, i, train_index);
					return TRUE;
				}
				if (track[node].type != NODE_EXIT && reservations->status[reverse_node][reverse_edge][i] == RSV_HELD) {
					//CommandOutput("reserve_conflict: %s(%d) %d %d", DATA->track[node].name, edge, i, train_index);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

int reserve(struct route_server_data * DATA, int train_index, int node, int edge) {
	struct reservation_data * reservations = &(DATA->reservations);
	int can = can_reserve(DATA, train_index, node, edge);
	if (can) {
		reservations->status[node][edge][train_index] = RSV_HELD;
	}
	return can;
}

void free_reservation(struct route_server_data * DATA, int train_index, int node, int edge) {
	struct reservation_data * reservations = &(DATA->reservations);
	reservations->status[node][edge][train_index] = RSV_NONE;
}

int reserve_for_node(struct route_server_data * DATA, int train_index, struct route_node * rnode, int edge, int length) {
	int status;
	if (rnode->length_reserved >= length) {
		return TRUE;
	}
	status = reserve(DATA, train_index, rnode->node, edge);
	if (status) {
		rnode->length_reserved = length;
		notify_reservation(DATA, train_index, rnode->node, edge, length);
	}
	return status;
}

//Give reservations along a secondary route for the given distance
void give_secondary_reservations(struct route_server_data * DATA, int train_index, int primary_index, int distance) {
	struct track_node * track = DATA->track;
	struct route_server_train_data * train_data = DATA->train_data + train_index;
	struct train_route * route = &(train_data->route);

	int i = 0, status = 0, dist = 0;
	int secondary_index = route->primary[primary_index].secondary_route_index;
	if (secondary_index < 0) {
		return;
	}
	struct route_node * secondary = route->secondary[secondary_index];
	int secondary_size = route->secondary_size[secondary_index];
	//Loop through the secondary route and add reservations until we reach the end, or we
	for (i = 0; i < secondary_size && dist < distance; i++) {
		dist += track[secondary[i].node].edge[secondary[i].edge].dist;
		if (track[secondary[i].node].type == NODE_EXIT) {
			return;
		}
		if (secondary[i].length_reserved <= 0) {
			status = reserve_for_node(DATA, train_index, secondary + i, secondary[i].edge,
					track[secondary[i].node].edge[secondary[i].edge].dist);
			if (!status) {
				return;
			}
		}
	}
}

//Try your best to give reservations to the train at train_index so that it can go at least
//distance before it has to stop.
void give_reservations(struct route_server_data * DATA, int train_index) {
	struct track_node * track = DATA->track;
	struct route_server_train_data * train_data = DATA->train_data + train_index;
	struct train_route * route = &(train_data->route);

	int i = primary_route_index(route, 0, train_data->last_pos.node);
	if (i < 0) {
		i = route->primary[0].node;
	}
	int dist = -train_data->last_pos.offset;
	int status;
	//Loop through the primary route until we find the node marking the end of the free section
	for (; i < route->primary_size && dist < train_data->requested_route_length; i++) {

		status = reserve_for_node(DATA, train_index, route->primary + i, route->primary[i].edge,
				track[route->primary[i].node].edge[route->primary[i].edge].dist);
		if (!status) {
			return;
		}
		if (route->primary[i].secondary_route_index >= 0) {
			give_secondary_reservations(DATA, train_index, i, train_data->requested_route_length - dist);
		}
		dist += track[route->primary[i].node].edge[route->primary[i].edge].dist;
		if (route->primary[i].reverse_here) {
			//Don't give reservations past a reverse edge
			return;
		}
	}
}

//Release reservations along the secondary route starting from the given node
void release_secondary_reservations(struct route_server_data * DATA, int train_index, int primary_index) {
	struct reservation_data * reservations = &(DATA->reservations);
	struct route_server_train_data * train_data = DATA->train_data + train_index;
	struct train_route * route = &(train_data->route);

	int i = 0;
	int secondary_index = route->primary[primary_index].secondary_route_index;
	if (secondary_index < 0) {
		return;
	}
	struct route_node * secondary = route->secondary[secondary_index];
	int secondary_size = route->secondary_size[secondary_index];
	//Loop through the secondary route and free all reservations
	for (i = 0; i < secondary_size; i++) {
		if (secondary[i].length_reserved > 0) {
			secondary[i].length_reserved = 0;
			reservations->status[secondary[i].node][secondary[i].edge][train_index] = RSV_NONE;
		}
	}
}

//Release reservations up to the given position on the given train's route
void release_reservations(struct route_server_data * DATA, int train_index, struct position * pos) {
	struct reservation_data * reservations = &(DATA->reservations);
	struct route_server_train_data * train_data = DATA->train_data + train_index;
	struct train_route * route = &(train_data->route);

	clear_extra_reservations(DATA, train_index);

	int i = 0;
	//Loop through the primary route until we find the node marking the end of the free section
	for (i = 0; i < route->primary_size; i++) {
		if (route->primary[i].node == pos->node) {
			return;
		}
		if (route->primary[i].length_reserved > 0
				|| reservations->status[route->primary[i].node][route->primary[i].edge][train_index] != RSV_NONE) {
			route->primary[i].length_reserved = 0;
			reservations->status[route->primary[i].node][route->primary[i].edge][train_index] = RSV_NONE;
			if (route->primary[i].secondary_route_index >= 0) {
				release_secondary_reservations(DATA, train_index, i);
			}
		}
	}
}

//Release reservations along the secondary route starting from the given node
void route_release_secondary_reservations(struct train_route * route, int primary_index) {
	int i = 0;
	int secondary_index = route->primary[primary_index].secondary_route_index;
	if (secondary_index < 0) {
		return;
	}
	struct route_node * secondary = route->secondary[secondary_index];
	int secondary_size = route->secondary_size[secondary_index];
	//Loop through the secondary route and free all reservations
	for (i = 0; i < secondary_size; i++) {
		if (secondary[i].length_reserved > 0) {
			secondary[i].length_reserved = 0;
		}
	}
}

//Release reservations up to the given position on the given train's route
void route_release_reservations(struct train_route * route, struct position * pos) {
	int i = 0;
	//Loop through the primary route until we find the node marking the end of the free section
	for (i = 0; i < route->primary_size; i++) {
		if (route->primary[i].node == pos->node) {
			return;
		}
		if (route->primary[i].length_reserved > 0) {
			route->primary[i].length_reserved = 0;
			if (route->primary[i].secondary_route_index >= 0) {
				route_release_secondary_reservations(route, i);
			}
		}
	}
}

void give_back_reservations(struct route_server_data * DATA, int train_index) {
	int i = 0, j = 0;
	struct train_route * route = &DATA->train_data[train_index].route;
	for (i = 0; i < route->primary_size; i++) {
		if (route->primary[i].length_reserved > 0) {
			reserve(DATA, train_index, route->primary[i].node, route->primary[i].edge);
			if (route->primary[i].secondary_route_index >= 0) {
				int secondary_index = route->primary[i].secondary_route_index;
				struct route_node * secondary = route->secondary[secondary_index];
				int secondary_size = route->secondary_size[secondary_index];
				//Loop through the secondary route and free all reservations
				for (j = 0; j < secondary_size; j++) {
					if (secondary[j].length_reserved > 0) {
						reserve(DATA, train_index, secondary[j].node, secondary[j].edge);
					}
				}
			}
		}
	}
}

void clear_all_reservations(struct route_server_data * DATA, int train_index) {
	int i = 0, k = 0;
	for (i = 0; i < TRACK_MAX; i++) {
		DATA->reservations.status[i][0][train_index] = RSV_NONE;
		DATA->reservations.status[i][1][train_index] = RSV_NONE;
	}
	for(i = 0; i < DATA->train_data[train_index].route.primary_size; i++) {
		DATA->train_data[train_index].route.primary[i].length_reserved = 0;
	}
	for(i = 0; i < DATA->train_data[train_index].route.num_secondary_routes; i++) {
		for(k = 0; k < DATA->train_data[train_index].route.secondary_size[i]; k++) {
			DATA->train_data[train_index].route.secondary[i][k].length_reserved = 0;
		}
	}
}

//Release all reservations for a train
void clear_reservations(struct route_server_data * DATA, int train_index) {
	struct position pos;
	pos.node = -1;
	pos.offset = 0;
	if (DATA->train_data[train_index].route.primary_size > 0) {
		release_reservations(DATA, train_index, &pos);
	}
}

//Release all reservations for a train
void clear_extra_reservations(struct route_server_data * DATA, int train_index) {
	struct train_route * route = &(DATA->train_data[train_index].route);
	int i = 0, j;
	short on_route[TRACK_MAX * 2];
	for (i = 0; i < TRACK_MAX * 2; i++) {
		on_route[i] = FALSE;
	}
	for (i = 0; i < route->primary_size; i++) {
		on_route[route->primary[i].node + (TRACK_MAX * route->primary[i].edge)] = TRUE;
	}
	for (i = 0; i < route->num_secondary_routes; i++) {
		for (j = 0; j < route->secondary_size[i]; j++) {
			on_route[route->secondary[i][j].node + (TRACK_MAX * route->secondary[i][j].edge)] = TRUE;
		}
	}
	for (i = 0; i < TRACK_MAX * 2; i++) {
		if ((!(on_route[i])) && DATA->reservations.status[i % TRACK_MAX][i / TRACK_MAX][train_index] == RSV_HELD) {
			DATA->reservations.status[i % TRACK_MAX][i / TRACK_MAX][train_index] = RSV_NONE;
		}
	}
	/*
	 clear_all_reservations(DATA, train_index);
	 give_back_reservations(DATA, train_index);
	 */
}

//Update reservation based on a message
void update_reservation(struct train_route *route, int node, int edge, int length_reserved) {
	int i = 0, j = 0;

	//Find all copies of the given node/edge in the route. We should never double back, so
	//updating all of them is OK
	for (i = 0; i < route->primary_size; i++) {
		if (route->primary[i].node == node && route->primary[i].edge == edge) {
			route->primary[i].length_reserved = length_reserved;
		}
	}
	//look along secondary route too
	for (i = 0; i < route->num_secondary_routes; i++) {
		for (j = 0; j < route->secondary_size[i]; j++) {
			if (route->secondary[i][j].node == node && route->secondary[i][j].edge == edge) {
				route->secondary[i][j].length_reserved = length_reserved;
			}
		}
	}
}

//Reserve where the train is now
void reserve_position(struct route_server_data * DATA, int train_index, struct position * pos) {
	struct position curpos = *pos; //= train_back(DATA->track, pos, CONFIG_TRAIN_MAX_LENGTH_FROM_PICKUP);
	//printf("curpos %s + %d\n", DATA->track[curpos.node].name, curpos.offset);

	int len = -curpos.offset;
	curpos.edge = track_expected_edge(DATA->track, curpos.node);
	curpos.offset = 0;
	while (len < CONFIG_TRAIN_MAX_LENGTH_FROM_PICKUP && DATA->track[curpos.node].type != NODE_EXIT) {
		reserve(DATA, train_index, curpos.node, curpos.edge);
		len += DATA->track[curpos.node].edge[curpos.edge].dist;
		curpos.node = DATA->track[curpos.node].edge[curpos.edge].dest->index;
		curpos.edge = track_expected_edge(DATA->track, curpos.node);
	}

	curpos = reverse_position(DATA->track, pos);
	//printf("curpos %s + %d\n", DATA->track[curpos.node].name, curpos.offset);

	len = -curpos.offset;
	//curpos.edge = track_expected_edge(DATA->track, curpos.node);
	curpos.offset = 0;
	while (len < CONFIG_TRAIN_MAX_LENGTH_FROM_PICKUP && DATA->track[curpos.node].type != NODE_EXIT) {
		reserve(DATA, train_index, curpos.node, curpos.edge);
		len += DATA->track[curpos.node].edge[curpos.edge].dist;
		curpos.node = DATA->track[curpos.node].edge[curpos.edge].dest->index;
		curpos.edge = track_expected_edge(DATA->track, curpos.node);
	}
}

//Reserve where the train is now
void reserve_position_sensor(struct route_server_data * DATA, int train_index, int sensor) {
	struct position curpos; //= train_back(DATA->track, pos, CONFIG_TRAIN_MAX_LENGTH_FROM_PICKUP);
	curpos.node = sensor;
	curpos.offset = 0;
	curpos.edge = 0;
	//printf("curpos %s + %d\n", DATA->track[curpos.node].name, curpos.offset);

	int len = 0;
	while (len < CONFIG_TRAIN_MAX_LENGTH_FROM_PICKUP && DATA->track[curpos.node].type != NODE_EXIT) {
		reserve(DATA, train_index, curpos.node, curpos.edge);
		len += DATA->track[curpos.node].edge[curpos.edge].dist;
		curpos.node = DATA->track[curpos.node].edge[curpos.edge].dest->index;
		curpos.edge = track_expected_edge(DATA->track, curpos.node);
	}

	curpos.node = DATA->track[sensor].reverse->index;
	curpos.offset = 0;
	curpos.edge = 0;
	//printf("curpos %s + %d\n", DATA->track[curpos.node].name, curpos.offset);

	len = 0;
	//curpos.edge = track_expected_edge(DATA->track, curpos.node);
	while (len < CONFIG_TRAIN_MAX_LENGTH_FROM_PICKUP && DATA->track[curpos.node].type != NODE_EXIT) {
		if (DATA->track[curpos.node].type == NODE_BRANCH) {
			reserve(DATA, train_index, curpos.node, 0);
			reserve(DATA, train_index, curpos.node, 1);
		} else {
			reserve(DATA, train_index, curpos.node, curpos.edge);
		}
		len += DATA->track[curpos.node].edge[curpos.edge].dist;
		curpos.node = DATA->track[curpos.node].edge[curpos.edge].dest->index;
		curpos.edge = track_expected_edge(DATA->track, curpos.node);
	}
}

