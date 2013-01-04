#include <userspace.h>


//max number of trains tab can display
#define MAX_TRAINS_DISPLAY 5
#define MAX_NODE_CHARACTERS 5
#define MAX_STATE_CHARACTERS 5

struct train_location_data {
	int num_trains;

	int train_nums[MAX_TRAINS_DISPLAY];

	volatile struct train_data * tr_data[MAX_TRAINS_DISPLAY];

	struct position tr_pos[MAX_TRAINS_DISPLAY];

	int tr_speed[MAX_TRAINS_DISPLAY];

	int tr_err[MAX_TRAINS_DISPLAY];

	int tr_max_err[MAX_TRAINS_DISPLAY];

	struct position tr_dest[MAX_TRAINS_DISPLAY];

	int tr_state[MAX_TRAINS_DISPLAY];

	int updated[MAX_TRAINS_DISPLAY][9];

	struct track_node track[TRACK_MAX];

	int tab_show;
	int periodic_running;
};

//helper functions
void draw_location_tab_outline( );
void draw_location_entry( struct train_location_data* DATA, int i );
void update_location_entry( struct train_location_data* DATA, int i );
int get_location_entry_num( struct train_location_data* DATA, int train_num );
void initialize_location_data( struct train_location_data* DATA );
void refresh_location_screen( struct train_location_data* DATA );
//end of helpers


void task_user_train_location_tab( ) {
	struct train_location_data data;
	struct train_location_data* DATA = &data;

	Delay( 50 );

	//initialize the display data
	initialize_location_data( DATA );

	//msgs
	struct two_number_message msg;
	struct number_message rpl;
	int recv_tid;

	//register as the dispatcher
	RegisterAs( "TRlocTAB" );
	CommandOutput("TRlocTAB tid: %d", MyTid());

	//register as a tab
	TabRegister( "TRAINS" );

	while( 1 ) {
		//get another message, we have reached the end
		Receive( &recv_tid, (char *)&msg, sizeof (msg) );

		//check message type and train tid
		switch( msg.message_type ) {
			case DELAY_WORKER_MESSAGE:
				//send the delay worker away
				Reply( recv_tid, (char *)0, 0 );

				//update each entry and write it
				refresh_location_screen( DATA );
				break;
			case TRAIN_OUTPUT_REGISTER_MESSAGE:
				rpl.message_type = ACK_MESSAGE;
				rpl.num = 0;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );

				//now take the message
				int index = get_location_entry_num( DATA, msg.num1 );
				if( index != -1 ) {
					//set the entry
					DATA->train_nums[index] = msg.num1;
					DATA->tr_data[index] = (struct train_data*)msg.num2;

					//grab the information from shared memory
					update_location_entry( DATA, index );

					//write the updated entry
					if( DATA->tab_show ) {
						draw_location_entry( DATA, index );
					}

					if( !DATA->periodic_running ) {
						WorkerPeriodicDelay( 50 );
						DATA->periodic_running = TRUE;
					}
				}
				break;
			case TAB_DISABLE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				DATA->tab_show = FALSE;
				break;
			case TAB_ENABLE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				DATA->tab_show = TRUE;
				clear_tab_space( );
				draw_location_tab_outline( );

				//set all entries to update, and update them
				int i = 0;
				for( i = 0; i < MAX_TRAINS_DISPLAY; i++ ) {
					if( DATA->train_nums[i] != -1 ) {
						int j = 0;
						for( j = 0; j < 9; j++ ) {
							DATA->updated[i][j] = 1;
						}
						draw_location_entry( DATA, i );
					}
				}
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


void refresh_location_screen( struct train_location_data* DATA ) {
	if( !DATA->tab_show ) {
		return;
	}

	int i = 0;
	for( i = 0; i < MAX_TRAINS_DISPLAY; i++ ) {
		if( DATA->train_nums[i] != -1 ) {
			update_location_entry( DATA, i );
			draw_location_entry( DATA, i );
		}
	}
}


void initialize_location_data( struct train_location_data* DATA ) {

	DATA->tab_show = FALSE;
	DATA->periodic_running = FALSE;

	DATA->num_trains = 0;

	int i = 0;
	for( i = 0; i < MAX_TRAINS_DISPLAY; i++ ) {
		DATA->train_nums[i] = -1;

		DATA->tr_data[i] = NULL;
		DATA->tr_speed[i] = 0;

		DATA->tr_pos[i].node = -1;
		DATA->tr_pos[i].offset = 0;

		DATA->tr_err[i] = 0;
		DATA->tr_max_err[i] = 0;
		DATA->tr_dest[i].node = -1;
		DATA->tr_dest[i].offset = 0;
		DATA->tr_state[i] = 0;

		int j = 0;
		for( j = 0; j < 9; j++ ) {
			DATA->updated[i][j] = 1;
		}
	}
}


void update_location_entry( struct train_location_data* DATA, int i ) {

	volatile struct train_data * tr_data = (volatile struct train_data *) DATA->tr_data[i];
	//train speed
	if( DATA->tr_speed[i] != tr_data->state.speed_target ) {
		DATA->tr_speed[i] = tr_data->state.speed_target;
		DATA->updated[i][1] = 1;
	}

	//pos node
	if( DATA->tr_pos[i].node != tr_data->state.current_pos_guess.node ) {
		DATA->tr_pos[i].node = tr_data->state.current_pos_guess.node;
		DATA->updated[i][2] = 1;
	}

	//pos offset
	if( DATA->tr_pos[i].offset != tr_data->state.current_pos_guess.offset ) {
		DATA->tr_pos[i].offset = tr_data->state.current_pos_guess.offset;
		DATA->updated[i][3] = 1;
	}

	//error
	if( DATA->tr_err[i] != tr_data->error_last ) {
		DATA->tr_err[i] = tr_data->error_last;
		DATA->updated[i][4] = 1;
	}

	//max error
	if( DATA->tr_max_err[i] != tr_data->error_max ) {
		DATA->tr_max_err[i] = tr_data->error_max;
		DATA->updated[i][5] = 1;
	}

	//dest node
	if( DATA->tr_dest[i].node != tr_data->dest.node ) {
		DATA->tr_dest[i].node = tr_data->dest.node;
		DATA->updated[i][6] = 1;
	}

	//dest offset
	if( DATA->tr_dest[i].offset != tr_data->dest.offset ) {
		DATA->tr_dest[i].offset = tr_data->dest.offset;
		DATA->updated[i][7] = 1;
	}

	//state
	if( DATA->tr_state[i] != tr_data->state.current ) {
		DATA->tr_state[i] = tr_data->state.current;
		DATA->updated[i][8] = 1;
	}
}


void draw_location_entry( struct train_location_data* DATA, int i ) {
	if( i < 0 || i >= MAX_TRAINS_DISPLAY || DATA->train_nums[i] == -1 ) {
		return;
	}

	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	int row = 7 + (2 * i);

	//turn off the cursor
	ap_putstr( &pbuff, "\x1B[?25l" );

	//train num
	if( DATA->updated[i][0] ) {
		ap_printf( &pbuff, "\x1B[%d;25f%d", row, DATA->train_nums[i] );
		DATA->updated[i][0] = 0;
	}

	//train speed
	if( DATA->updated[i][1] ) {
		ap_printf( &pbuff, "\x1B[%d;29f  \x1B[%d;29f%d", row, row, DATA->tr_speed[i] );
		DATA->updated[i][1] = 0;
	}

	//pos node
	if( DATA->updated[i][2] ) {
		if( DATA->tr_pos[i].node < 0 ) {
			if(DATA->tr_data[i]->state.lost) {
				ap_printf( &pbuff, "\x1B[%d;33fLOST ", row );
			} else {
				ap_printf( &pbuff, "\x1B[%d;33f-1   ", row );
			}
		} else {
			ap_printf( &pbuff, "\x1B[%d;33f     \x1B[%d;33f%s", row, row, DATA->tr_data[i]->track[DATA->tr_pos[i].node].name );
		}
		DATA->updated[i][2] = 0;
	}

	//pos offset
	if( DATA->updated[i][3] ) {
		if( DATA->tr_pos[i].node < 0 ) {
			ap_printf( &pbuff, "\x1B[%d;40f --  ", row );
		} else if( DATA->tr_pos[i].offset > -9999 && DATA->tr_pos[i].offset < 99999 ) {
			ap_printf( &pbuff, "\x1B[%d;40f     \x1B[%d;40f%d", row, row, DATA->tr_pos[i].offset );
		} else {
			ap_printf( &pbuff, "\x1B[%d;40fOVER ", row );
		}
		DATA->updated[i][3] = 0;
	}

	//error
	if( DATA->updated[i][4] ) {
		if( DATA->tr_err[i] > -999 && DATA->tr_err[i] < 9999 ) {
			ap_printf( &pbuff, "\x1B[%d;47f    \x1B[%d;47f%d", row, row, DATA->tr_err[i] );
		} else {
			ap_printf( &pbuff, "\x1B[%d;47fOVER", row );
		}
		DATA->updated[i][4] = 0;
	}

	//max error
	if( DATA->updated[i][5] ) {
		if( DATA->tr_max_err[i] > -9999 && DATA->tr_max_err[i] < 99999 ) {
			ap_printf( &pbuff, "\x1B[%d;53f     \x1B[%d;53f%d", row, row, DATA->tr_max_err[i] );
		} else {
			ap_printf( &pbuff, "\x1B[%d;53fOVER ", row );
		}
		DATA->updated[i][5] = 0;
	}

	//dest node
	if( DATA->updated[i][6] ) {
		if( DATA->tr_dest[i].node < 0 ) {
			ap_printf( &pbuff, "\x1B[%d;60fN/A  ", row );
		} else {
			ap_printf( &pbuff, "\x1B[%d;60f     \x1B[%d;60f%s", row, row, DATA->tr_data[i]->track[DATA->tr_dest[i].node].name );
		}
		DATA->updated[i][6] = 0;
	}

	//dest offset
	if( DATA->updated[i][7] ) {
		if( DATA->tr_dest[i].node < 0 ) {
			ap_printf( &pbuff, "\x1B[%d;67f -- ", row );
		} else if( DATA->tr_dest[i].offset > -999 && DATA->tr_dest[i].offset < 9999 ) {
			ap_printf( &pbuff, "\x1B[%d;67f    \x1B[%d;67f%d", row, row, DATA->tr_dest[i].offset );
		} else {
			ap_printf( &pbuff, "\x1B[%d;67fOVER", row );
		}
		DATA->updated[i][7] = 0;
	}

	// state
	if( DATA->updated[i][8] ) {
		//ap_printf( &pbuff, "\x1B[%d;73f     \x1B[%d;73f%s", row, row, DATA->train_state[i] );
		ap_printf( &pbuff, "\x1B[%d;73f     \x1B[%d;73f%d", row, row, DATA->tr_state[i] );
		DATA->updated[i][8] = 0;
	}

	//reload cursor position, and enable it
	ap_putstr( &pbuff, "\x1B[u\x1B[?25h" );

	//send the print
	Putbuff( COM2, &pbuff );

}


int get_location_entry_num( struct train_location_data* DATA, int train_num ) {
	int i = 0;
	for( i = 0; i < MAX_TRAINS_DISPLAY; i++ ) {
		if( DATA->train_nums[i] == train_num || DATA->train_nums[i] == -1 ) {
			return i;
		} else if( DATA->train_nums[i] == -1 ) {
			DATA->train_nums[i] = train_num;
			DATA->updated[i][0] = 1;
			DATA->num_trains++;
			return i;
		}
	}
	return -1;
}


void draw_location_tab_outline( ) {
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	/* draw the outlines */

	ap_putstr( &pbuff, "\x1B[?25l\x1B[36m" ); //turn cursor off andset attributes

	ap_putstr( &pbuff, "\x1B[5;27f|   |      |      |     |      |      |     |" );
	ap_putstr( &pbuff, "\x1B[7;27f|   |      |      |     |      |      |     |" );
	ap_putstr( &pbuff, "\x1B[9;27f|   |      |      |     |      |      |     |" );
	ap_putstr( &pbuff, "\x1B[11;27f|   |      |      |     |      |      |     |" );
	ap_putstr( &pbuff, "\x1B[13;27f|   |      |      |     |      |      |     |" );
	ap_putstr( &pbuff, "\x1B[15;27f|   |      |      |     |      |      |     |" );

	ap_putstr( &pbuff, "\x1B[6;24f---|---|------|------|-----|------|------|-----|-------" );
	ap_putstr( &pbuff, "\x1B[8;24f---|---|------|------|-----|------|------|-----|-------" );
	ap_putstr( &pbuff, "\x1B[10;24f---|---|------|------|-----|------|------|-----|-------" );
	ap_putstr( &pbuff, "\x1B[12;24f---|---|------|------|-----|------|------|-----|-------" );
	ap_putstr( &pbuff, "\x1B[14;24f---|---|------|------|-----|------|------|-----|-------" );

	ap_putstr( &pbuff, "\x1B[0m" ); //clear attributes

	/* finished outlines */

	/* draw the titles */
	ap_putstr( &pbuff, "\x1B[1;37m" ); //set attributes

	ap_putstr( &pbuff, "\x1B[5;25f#" );
	ap_putstr( &pbuff, "\x1B[5;29fV" );
	ap_putstr( &pbuff, "\x1B[5;33fNODE" );
	ap_putstr( &pbuff, "\x1B[5;40fOFF" );
	ap_putstr( &pbuff, "\x1B[5;47fERR" );
	ap_putstr( &pbuff, "\x1B[5;53fMAXE" );
	ap_putstr( &pbuff, "\x1B[5;60fDEST" );
	ap_putstr( &pbuff, "\x1B[5;67fOFF" );
	ap_putstr( &pbuff, "\x1B[5;73fSTATE" );

	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" ); //clear attributes and restore cursor
	/* finished titles */

	Putbuff( COM2, &pbuff );
}

