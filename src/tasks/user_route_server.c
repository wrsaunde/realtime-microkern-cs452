#include <userspace.h>

void task_user_route_server() {
	int tid = Create(0,&task_system_extra_stack);
	assert(tid == MyTid() + 1,"tid == MyTid() + 1");

	RegisterAs("routeserv");

	//Struct to hold data of the route server
	struct route_server_data data;
	struct route_server_data* DATA = &data;

	int i, train_index;

	//Message structures
	struct train_route_request_message buffer;
	struct empty_message rpl;
	struct empty_message route_message;
	struct train_route_request_message *route_request_message;
	struct train_reservation_request_message *res_req_msg;
	struct empty_message *emptymsg;
	struct two_number_message * twomsg;

	struct position pos;

	init_route_server_data(DATA);

	int tabtid = WhoIs("resTAB");
	while(tabtid < 0) {
		Delay(2);
		tabtid = WhoIs("resTAB");
	}

	struct number_message nummsg;
	nummsg.message_type = RESERVATION_TAB_INIT_MESSAGE;
	nummsg.num = (int) DATA;
	Send(tabtid, (char *) &nummsg, sizeof(nummsg), (char *) &rpl, sizeof(rpl));

	while (1) {
		int recv_size = 0;
		int recv_tid = -1;
		//get the next request
		//CommandOutput("[ROUTESERV] Leaving Route Server");
		//Delay(10);
		recv_size = Receive(&recv_tid, (char *) &buffer, sizeof(buffer));
		emptymsg = (struct empty_message *) &buffer;

		//CommandOutput("[ROUTESERV] Entering Route Server message_type: %d, recv_tid: %d, recv_size: %d", emptymsg->message_type, recv_tid, recv_size);
		//Delay(10);

		switch (emptymsg->message_type) {
		case TRAIN_ROUTE_REQUEST_MESSAGE:
			//Message requesting that a specific train be given a new route
			//CommandOutput("[ROUTESERV] Got route request message");
			//Reply immediately
			rpl.message_type = ACK_MESSAGE;
			Reply(recv_tid, (char *) 0, 0);

			if(recv_size < sizeof(struct train_route_request_message)) {
				break;
			}

			route_request_message = (struct train_route_request_message *) &buffer;
			//Get index
			train_index = get_train_index(DATA, route_request_message->train);

			//CommandOutput("Conflict with D14: %d", reserve_conflict(DATA, train_index, 0x3d, 0));
			//Delay(300);
			//Update tid
			DATA->train_data[train_index].tid = route_request_message->train_tid;

			//Do some checks on the requested route
			if (route_request_message->pos1.node < 0 || route_request_message->pos1.node >= TRACK_MAX
					|| DATA->track[route_request_message->pos1.node].type == NODE_NONE) {
				CommandOutput("[ROUTESERV] Train is lost, can't route it :(");
			} else if (route_request_message->pos1.offset < 0 || route_request_message->pos2.node < 0
					|| route_request_message->pos1.node >= TRACK_MAX
					|| DATA->track[route_request_message->pos1.node].type == NODE_NONE
					|| route_request_message->pos1.offset < 0) {
				CommandOutput("[ROUTESERV] Bad route, can't direct train");

			} else {

				CommandOutput("[ROUTESERV] Routing train %d (tid %d) from %s + %d mm to %s + %d mm",
						route_request_message->train, route_request_message->train_tid,
						DATA->track[route_request_message->pos1.node].name, route_request_message->pos1.offset,
						DATA->track[route_request_message->pos2.node].name, route_request_message->pos2.offset);

				//Delay(10);
				//Get new route
				new_route(DATA, route_request_message->train, &(route_request_message->pos1),
						&(route_request_message->pos2));

				//CommandOutput("[ROUTESERV] Got route, copying back to train");
				//Delay(10);

				//Create message to send to train to give it the new route
				route_message.message_type = TRAIN_ROUTE_MESSAGE;

				//Copything
				copy_route(&(DATA->train_data[train_index].route),route_request_message->route);
				/*mem_copy((char*) &(DATA->train_data[train_index].route), (char*) route_request_message->route,
						sizeof(struct train_route));*/

				//CourierSend(route_request_message->train_tid, (char*) &route_message, sizeof(route_message));
				//CommandOutput("[ROUTESERV] Copied route, sending back to train");
				//Delay(10);

				Send(route_request_message->train_tid, (char*) &route_message, sizeof(route_message), (char*) &rpl,
						sizeof(rpl));

				DATA->train_data[train_index].requested_route_length = route_request_message->requested_route_length;
				DATA->train_data[train_index].last_pos.node = route_request_message->pos1.node;
				DATA->train_data[train_index].last_pos.offset = route_request_message->pos1.offset;

				//CommandOutput("[ROUTESERV] Giving reservations to train");
				//Delay(10);

				give_reservations(DATA, train_index);

			}
			break;

		case TRAIN_ROUTE_RESERVATION_REQUEST_MESSAGE:
			//Message from a train requesting that it be given MOAR RESERVATION DAKKA

			//Reply immediately
			rpl.message_type = ACK_MESSAGE;
			Reply(recv_tid, (char *) &rpl, sizeof(rpl));

			res_req_msg = (struct train_reservation_request_message *) &buffer;
			/*
			CommandOutput("[ROUTESERV] Got a request to give more reservation to train %d for %d from %s(%d) + %dmm",
					res_req_msg->train, res_req_msg->requested_route_length, DATA->track[res_req_msg->pos.node].name,
					res_req_msg->pos.node, res_req_msg->pos.offset);
			*/

			if (res_req_msg->pos.node >= 0) {
				train_index = get_train_index(DATA, res_req_msg->train);

				DATA->train_data[train_index].requested_route_length = res_req_msg->requested_route_length;
				DATA->train_data[train_index].last_pos = res_req_msg->pos;

				give_reservations(DATA, train_index);
			}
			break;
		case TRAIN_ROUTE_FREE_RESERVATION_MESSAGE:
			//Request to free reservations

			//Reply immediately
			rpl.message_type = ACK_MESSAGE;
			Reply(recv_tid, (char *) &rpl, sizeof(rpl));

			twomsg = (struct two_number_message *) &buffer;
			CommandOutput("[ROUTESERV] Got a request to release reservations from train %d (sensor %d)", twomsg->num1,
					twomsg->num2);

			train_index = get_train_index(DATA, twomsg->num1);

			pos.node = twomsg->num2;
			pos.offset = 0;

			release_reservations(DATA, train_index, &pos);
			break;
		case TRAIN_ROUTE_RESERVE_WHERE_I_AM_MESSAGE:
			//Request to please reserve where I am, thank you
			//Reply immediately
			rpl.message_type = ACK_MESSAGE;
			Reply(recv_tid, (char *) &rpl, sizeof(rpl));

			twomsg = (struct two_number_message *) &buffer;
			CommandOutput("[ROUTESERV] Got a reserve current location train %d (sensor %d)", twomsg->num1,
					DATA->track[twomsg->num2].name);

			train_index = get_train_index(DATA, twomsg->num1);
			clear_all_reservations(DATA, train_index);
			CommandOutput("Getting Reservation for Current Position");
			reserve_position_sensor(DATA, train_index, twomsg->num2);
			break;
		case TRAIN_ROUTE_RESERVE_WHERE_I_AM_POS_MESSAGE:
			//Request to please reserve where I am, thank you
			//Reply immediately
			rpl.message_type = ACK_MESSAGE;
			Reply(recv_tid, (char *) &rpl, sizeof(rpl));
			struct position_and_train_message * ptmsg;
			ptmsg = (struct position_and_train_message *) &buffer;
			CommandOutput("[ROUTESERV] Got a reserve current location train %d (sensor %d)", ptmsg->train_number,
					DATA->track[ptmsg->pos.node].name, ptmsg->pos.offset);
			train_index = get_train_index(DATA, ptmsg->train_number);
			clear_all_reservations(DATA, train_index);
			CommandOutput("Getting Reservation for Current Position");
			reserve_position(DATA, train_index, &ptmsg->pos);
			break;
		case TRAIN_ROUTE_DEBUG_RESERVE_MESSAGE:
			Reply(recv_tid, (char *) 0, 0);
			twomsg = (struct two_number_message *) &buffer;

			train_index = get_train_index(DATA, CONFIG_ROUTE_SERVER_DEBUG_TRAIN);
			reserve(DATA, train_index, twomsg->num1, twomsg->num2);
			break;
		case TRAIN_ROUTE_DEBUG_FREE_MESSAGE:
			Reply(recv_tid, (char *) 0, 0);
			twomsg = (struct two_number_message *) &buffer;
			train_index = get_train_index(DATA, CONFIG_ROUTE_SERVER_DEBUG_TRAIN);
			free_reservation(DATA, train_index, twomsg->num1, twomsg->num2);
			break;
		default:
			rpl.message_type = INVALID_OPERATION_MESSAGE;
			Reply(recv_tid, (char *) &rpl, sizeof(rpl));
			break;
		}

		//CommandOutput("[ROUTESERV] Giving reservations to everyone!");
		//Delay(10);

		//Give reservations to all trains
		for (i = 0; i < DATA->num_trains; i++) {
			if(DATA->train_data[i].requested_route_length > 0 && DATA->train_data[i].route.primary_size > 0) {
				give_reservations(DATA, i);
			}
		}

	}
}
