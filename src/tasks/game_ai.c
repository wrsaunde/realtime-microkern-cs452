#include <userspace.h>

#define STATE_WAITING 0
#define STATE_ROUTING 1
#define STATE_REPAIRING 2

struct game_ai_data {
	//the information about the targets
	struct game_target_graph targ_graph;
	struct game_target_graph* TARG_GRAPH;

	int num_trains;
	int train_nums[5];
	int train_repair_targ[5];
	int state[5];
	volatile struct train_data * trains[5];

	//queue for destroyed targets
	int ibuff1[GAME_NUM_TARGETS];
	struct int_buffer destroyed_q;

	//queue for routeable trains
	int ibuff2[5];
	struct int_buffer ready_tr_q;

};

void initialize_game_ai_data( struct game_ai_data* DATA );
void add_destroyed( struct game_ai_data* DATA, int id );
void add_ready_train( struct game_ai_data* DATA, int id );
int find_tr_index( struct game_ai_data* DATA, int tnum );
void RouteTrainsToDestroyed( struct game_ai_data* DATA );
void restart_game( struct game_ai_data* DATA );


void task_game_ai( ) {
	struct game_ai_data data;
	struct game_ai_data* DATA = &data;

	Delay( 50 );

	//initialize the display data
	initialize_game_ai_data( DATA );

	//msgs
	struct two_number_message msg;
	struct number_message rpl;
	int recv_tid;

	//register as the dispatcher
	RegisterAs( "gameAI" );

	while( 1 ) {
		//get another message, we have reached the end
		Receive( &recv_tid, (char *)&msg, sizeof (msg) );

		//check message type and train tid
		switch( msg.message_type ) {
			case GAME_AI_REGISTER_TRAIN:
				rpl.message_type = ACK_MESSAGE;
				rpl.num = 0;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );

				DATA->train_nums[DATA->num_trains] = msg.num1;
				DATA->state[DATA->num_trains] = STATE_WAITING;
				add_ready_train( DATA, DATA->num_trains );
				DATA->trains[DATA->num_trains++] = (struct train_data*)msg.num2;

				break;
			case GAME_DESTROYED_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				//add to the queue of targets that have been destroyed
				add_destroyed( DATA, msg.num1 );
				break;
			case DELAY_WORKER_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );

				//we are finished repairing a target, tell the game tab
				struct two_number_message send_msg;
				send_msg.message_type = GAME_REPAIRED_MESSAGE;
				send_msg.num1 = DATA->train_repair_targ[msg.num1];
				DATA->train_repair_targ[msg.num1] = -1;
				Send( WhoIs( "gameTAB" ), (char*)&send_msg, sizeof (send_msg), (char*)0, 0 );

				//add the train to the ready trains
				DATA->state[msg.num1] = STATE_WAITING;
				add_ready_train( DATA, msg.num1 );
				break;
			case GAME_FINISHED_ROUTE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );

				int index = find_tr_index( DATA, msg.num1 );
				//we are at the target, start repairing
				if( DATA->state[index] == STATE_ROUTING ) {
					struct two_number_message send_msg;
					send_msg.message_type = GAME_REPAIRING_MESSAGE;
					send_msg.num1 = DATA->train_repair_targ[index];
					DATA->state[index] = STATE_REPAIRING;
					Send( WhoIs( "gameTAB" ), (char*)&send_msg, sizeof (send_msg), (char*)0, 0 );
					WorkerDelayUntilWithId( Time( ) + GAME_REPAIR_TIME, index );
				}
				break;
			case GAME_WON_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				restart_game( DATA );
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				rpl.num = 0;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );
				break;
		}

		RouteTrainsToDestroyed( DATA );
	}

	Exit( );
}


void initialize_game_ai_data( struct game_ai_data* DATA ) {

	//initialize the target graph
	DATA->TARG_GRAPH = &DATA->targ_graph;
	initialize_targets( DATA->TARG_GRAPH );

	DATA->num_trains = 0;

	intbuffer_init( &DATA->destroyed_q, DATA->ibuff1, GAME_NUM_TARGETS );
	intbuffer_init( &DATA->ready_tr_q, DATA->ibuff2, 5 );

}


void restart_game( struct game_ai_data* DATA ) {
	int i = 0;

	//empty the queues
	intbuffer_init( &DATA->destroyed_q, DATA->ibuff1, GAME_NUM_TARGETS );
	intbuffer_init( &DATA->ready_tr_q, DATA->ibuff2, 5 );

	for( i = 0; i < DATA->num_trains; i++ ) {
		add_ready_train( DATA, i );
		DATA->train_repair_targ[i] = -1;
		DATA->state[i] = STATE_WAITING;
	}
}


void add_destroyed( struct game_ai_data* DATA, int id ) {
	intbuffer_push( &DATA->destroyed_q, id );
}


void add_ready_train( struct game_ai_data* DATA, int id ) {
	intbuffer_push( &DATA->ready_tr_q, id );
}


int find_tr_index( struct game_ai_data* DATA, int tnum ) {
	int i = 0;
	for( i = 0; i < DATA->num_trains; i++ ) {
		if( tnum == DATA->train_nums[i] ) {
			return i;
		}
	}
	return 0;
}


void RouteTrainsToDestroyed( struct game_ai_data* DATA ) {

	while( DATA->ready_tr_q.state == INTBUFF_NOT_EMPTY && DATA->destroyed_q.state == INTBUFF_NOT_EMPTY ) {
		int trindex = intbuffer_pop( &DATA->ready_tr_q );
		int targindex = intbuffer_pop( &DATA->destroyed_q );

		//route the train
		DATA->state[trindex] = STATE_ROUTING;
		DATA->train_repair_targ[trindex] = targindex;

		struct train_route_request_message rrmsg;
		rrmsg.message_type = TRAIN_ROUTE_REQUEST_MESSAGE;
		rrmsg.train = DATA->trains[trindex]->train_number;
		rrmsg.train_tid = DATA->trains[trindex]->tid_me;
		rrmsg.pos1.node = 0;
		rrmsg.pos1.offset = 0;
		rrmsg.pos2.node = DATA->TARG_GRAPH->targets[targindex].track_position.node;
		rrmsg.pos2.offset = DATA->TARG_GRAPH->targets[targindex].track_position.offset;
		Send( DATA->trains[trindex]->tid_me, (char*)&rrmsg, sizeof (rrmsg), (char*)0, 0 );
	}
}
