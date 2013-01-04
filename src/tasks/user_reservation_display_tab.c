#include <userspace.h>


//max number of trains tab can display
#define MAX_TRAINS_DISPLAY 2

//how far do we go to get to second train
#define T_OFF_SIZE 28

//how far for seconday
#define T_OFF_SEC 14

struct reservation_tab_data {
	volatile struct route_server_data* rs_data;

	int tab_show;
};

//helper functions
void initialize_reservation_data( struct reservation_tab_data* DATA );
void draw_reservation_outline( );
//end of helpers

//output helpers
void output_reservation( struct reservation_tab_data* DATA );
void output_train( struct reservation_tab_data* DATA, int tnum );

void clear_reserve_line( struct reservation_tab_data* DATA, struct print_buffer* pbuff, int tnum, int sec, int line );
void print_reserve_line( struct reservation_tab_data* DATA, struct print_buffer* pbuff, int tnum, int sec, int line, char* n1, char* n2 );
//end of output helpers


void task_user_reservation_display_tab( ) {
	struct reservation_tab_data data;
	struct reservation_tab_data* DATA = &data;

	Delay(50);

	//register
	RegisterAs( "resTAB" );

	CommandOutput("resTAB tid: %d", MyTid());

	//initialize the display data
	initialize_reservation_data( DATA );

	//msgs
	struct train_position_output_message msg;
	struct number_message rpl;
	int recv_tid;

	//register as a tab
	TabRegister( "RESERVATION" );

	//create a periodic delay worker so
	//we print at regular intervals
	WorkerPeriodicDelay( CONFIG_UI_RESERVATION_REFRESH_RATE );

	while( 1 ) {
		//get another message, we have reached the end
		Receive( &recv_tid, (char *)&msg, sizeof (msg) );

		//check message type and train tid
		switch( msg.message_type ) {
			case TAB_DISABLE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				DATA->tab_show = FALSE;
				break;
			case TAB_ENABLE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				DATA->tab_show = TRUE;
				clear_tab_space( );
				draw_reservation_outline( );
				output_reservation( DATA );
				break;
			case DELAY_WORKER_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				output_reservation( DATA );
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				rpl.num = 0;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );
				break;
		}
	}

	Exit( );
}


void output_reservation( struct reservation_tab_data* DATA ) {
	if( !DATA->tab_show ) {
		//if we aren't showing, don't bother displaying
		return;
	}
	int tnum = 0;

	while( tnum < DATA->rs_data->num_trains && tnum < 2 ) {
		output_train( DATA, tnum );
		tnum++;
	}

}


void output_train( struct reservation_tab_data* DATA, int tnum ) {
	struct print_buffer pbuff;
	volatile struct train_route * route = &(DATA->rs_data->train_data[tnum].route);
	struct track_node * track = (DATA->rs_data->track);
	int found = FALSE;
	ap_init_buff( &pbuff );
	ap_putstr( &pbuff, "\x1B[?25l" );
	int i = 0, j = 0, k = 0, e = 0, primary_line = 0, secondary_line = 0, s_index = 0;


	//output the train number
	ap_printf( &pbuff, "\x1B[%d;%df%d", 5, (27 + tnum * T_OFF_SIZE) + 1, DATA->rs_data->train_data[tnum].train_number );

	for(i = 0; i < TRACK_MAX; i++) {
		for(e = 0; e < 2; e++) {
			if(*((volatile int *) DATA->rs_data->reservations.status[i][e] + tnum) == RSV_HELD) {
				found = FALSE;
				for(j = 0; j < route->primary_size; j++) {
					volatile struct route_node * primary_node = route->primary + j;
					if(primary_node->node == i && primary_node->edge == e) {
						found = TRUE;
						break;
					}
					s_index = route->primary[j].secondary_route_index;
					if(s_index >= 0) {
						for(k = 0; k < route->secondary_size[s_index]; k++) {
							volatile struct route_node * secondary_node = route->secondary[s_index] + k;
							if(secondary_node->node == i && secondary_node->edge == e) {
								found = TRUE;
								break;
							}
						}
					}
				}
				if(found == FALSE) {
					print_reserve_line(DATA, &pbuff, tnum, FALSE, primary_line, track[i].name, track[i].edge[0].dest->name);
					primary_line++;
				}
			}

		}
	}

	for(i = 0; i < route->primary_size; i++) {
		if(route->primary[i].length_reserved > 0) {
			print_reserve_line(DATA, &pbuff, tnum, FALSE, primary_line, track[route->primary[i].node].name, track[route->primary[i].node].edge[route->primary[i].edge].dest->name);
			primary_line++;
		}
		s_index = route->primary[i].secondary_route_index;
		if(s_index >= 0) {
			for(k = 0; k < route->secondary_size[s_index]; k++) {
				if(route->secondary[s_index][k].length_reserved > 0) {
					print_reserve_line(DATA, &pbuff, tnum, TRUE, secondary_line, track[route->secondary[s_index][k].node].name, track[route->secondary[s_index][k].node].edge[route->secondary[s_index][k].edge].dest->name);
					secondary_line++;
				}
			}
		}
	}
	/*
	if(DATA->rs_data->train_data[tnum].train_number == CONFIG_ROUTE_SERVER_DEBUG_TRAIN) {
		for(i = 0; i < TRACK_MAX; i++) {
			if(DATA->rs_data->reservations.status[i][0][tnum] == RSV_HELD) {
				print_reserve_line(DATA, &pbuff, tnum, FALSE, primary_line, track[i].name, track[i].edge[0].dest->name);
				primary_line++;
			}
			if(DATA->rs_data->reservations.status[i][1][tnum] == RSV_HELD) {
				print_reserve_line(DATA, &pbuff, tnum, FALSE, primary_line, track[i].name, track[i].edge[1].dest->name);
				primary_line++;
			}
		}
	}*/
	while(primary_line <= 10) {
		clear_reserve_line(DATA, &pbuff, tnum, FALSE, primary_line);
		primary_line++;
	}
	while(secondary_line <= 10) {
		clear_reserve_line(DATA, &pbuff, tnum, TRUE, secondary_line);
		secondary_line++;
	}
	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" ); //clear attributes and restore cursor
	Putbuff( COM2, &pbuff );
}


void print_reserve_line( struct reservation_tab_data* DATA, struct print_buffer* pbuff, int tnum, int sec, int line, char* n1, char* n2 ) {
	if( line > 10 ) {
		return;
	}

	int spaces = 11 - strlen( n1 ) - strlen( n2 );

	if( sec ) {
		ap_printf( pbuff, "\x1B[%d;%df%s:%s", 6 + line, (25 + T_OFF_SEC + tnum * T_OFF_SIZE), n1, n2 );
	} else {
		ap_printf( pbuff, "\x1B[%d;%df%s:%s", 6 + line, (25 + tnum * T_OFF_SIZE), n1, n2 );
	}
	int i = 0;
	for( i = 0; i < spaces; i++ ) {
		ap_putc( pbuff, ' ' );
	}
}


void clear_reserve_line( struct reservation_tab_data* DATA, struct print_buffer *pbuff, int tnum, int sec, int line ) {
	if( line > 10 ) {
		return;
	}

	if( sec ) {
		ap_printf( pbuff, "\x1B[%d;%df           ", 6 + line, (25 + T_OFF_SEC + tnum * T_OFF_SIZE));
	} else {
		ap_printf( pbuff, "\x1B[%d;%df           ", 6 + line, (25 + tnum * T_OFF_SIZE));
	}
}


void initialize_reservation_data( struct reservation_tab_data* DATA ) {

	DATA->tab_show = FALSE;
	DATA->rs_data = NULL;

	//get the initialization message from the route server
	int tid;
	struct number_message init_msg;
	Receive( &tid, (char *)&init_msg, sizeof (init_msg) );
	Reply( tid, (char *)0, 0 );
	DATA->rs_data = (struct route_server_data*)init_msg.num;
}


void draw_reservation_outline( ) {
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	ap_putstr( &pbuff, "\x1B[?25l\x1B[36m" ); //turn cursor off and set attributes

	int row = 0;
	for( row = 5; row <= 16; row++ ) {
		ap_printf( &pbuff, "\x1B[%d;37f|\x1B[%d;51f|\x1B[%d;65f|", row, row, row );
	}

	ap_putstr( &pbuff, "\x1B[0m" ); //clear attributes
	ap_putstr( &pbuff, "\x1B[31;1m" ); //set attributes
	ap_printf( &pbuff, "\x1B[5;25fTR     PRIM\x1B[5;47fSEC\x1B[5;53fTR     PRIM\x1B[5;75fSEC" );

	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" ); //clear attributes and restore cursor
	/* finished titles */

	Putbuff( COM2, &pbuff );
}




