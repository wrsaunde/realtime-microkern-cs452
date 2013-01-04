#include <userspace.h>


//max number of trains tab can display
#define MAX_TRAINS_DISPLAY 5
#define MAX_NODE_CHARACTERS 5
#define MAX_STATE_CHARACTERS 5

struct train_display_data {
	int num_trains;

	int train_nums[MAX_TRAINS_DISPLAY];
	int train_speeds[MAX_TRAINS_DISPLAY];
	char train_pos_node[MAX_TRAINS_DISPLAY][MAX_NODE_CHARACTERS + 1];
	int train_pos_off[MAX_TRAINS_DISPLAY];
	int train_err[MAX_TRAINS_DISPLAY];
	int train_max_err[MAX_TRAINS_DISPLAY];
	char train_dest_node[MAX_TRAINS_DISPLAY][MAX_NODE_CHARACTERS + 1];
	int train_dest_off[MAX_TRAINS_DISPLAY];
	char train_state[MAX_TRAINS_DISPLAY][MAX_STATE_CHARACTERS + 1];

	int updated[MAX_TRAINS_DISPLAY][9];

	int tab_show;
};

//helper functions
void draw_tab_outline( );
void draw_entry( struct train_display_data* DATA, int i );
void update_entry( struct train_display_data* DATA, int i, struct train_position_output_message* msg );
int get_entry_num( struct train_display_data* DATA, int train_num );
void initialize_display_data( struct train_display_data* DATA );
//end of helpers


void task_user_train_display_tab( ) {
	struct train_display_data data;
	struct train_display_data* DATA = &data;

	Delay(50);

	//initialize the display data
	initialize_display_data( DATA );

	//msgs
	struct train_position_output_message msg;
	struct number_message rpl;
	int recv_tid;

	//register as the dispatcher
	RegisterAs( "trainsTAB" );

	CommandOutput("trainsTAB tid: %d", MyTid());

	//register as a tab
	TabRegister( "TRAINS" );

	while( 1 ) {
		//get another message, we have reached the end
		Receive( &recv_tid, (char *)&msg, sizeof (msg) );

		//check message type and train tid
		switch( msg.message_type ) {
			case TRAIN_POSITION_OUTPUT_MESSAGE:
				rpl.message_type = ACK_MESSAGE;
				rpl.num = 0;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );

				//now take the message
				int index = get_entry_num( DATA, msg.train_nums );
				if( index != -1 ) {
					//copy the changes to the store
					update_entry( DATA, index, &msg );

					//write the updated entry
					if( DATA->tab_show ) {
						draw_entry( DATA, index );
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
				clear_tab_space();
				draw_tab_outline();

				//set all entries to update, and update them
				int i = 0;
				for( i = 0; i < MAX_TRAINS_DISPLAY; i++ ) {
					if(DATA->train_nums[i] != -1){
						int j = 0;
						for( j = 0; j < 9; j++ ) {
							DATA->updated[i][j] = 1;
						}
						draw_entry( DATA, i );
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


void initialize_display_data( struct train_display_data* DATA ) {

	DATA->tab_show = FALSE;

	DATA->num_trains = 0;

	int i = 0;
	for( i = 0; i < MAX_TRAINS_DISPLAY; i++ ) {
		DATA->train_nums[i] = -1;
		DATA->train_speeds[i] = 0;
		DATA->train_pos_node[i][0] = '\0';
		DATA->train_pos_off[i] = 0;
		DATA->train_err[i] = 0;
		DATA->train_max_err[i] = 0;
		;
		DATA->train_dest_node[i][0] = '\0';
		DATA->train_dest_off[i] = 0;
		DATA->train_state[i][0] = '\0';

		int j = 0;
		for( j = 0; j < 9; j++ ) {
			DATA->updated[i][j] = 1;
		}
	}
}


void update_entry( struct train_display_data* DATA, int i, struct train_position_output_message* msg ) {
	//train speed
	if( DATA->train_speeds[i] != msg->train_speeds ) {
		DATA->train_speeds[i] = msg->train_speeds;
		DATA->updated[i][1] = 1;
	}

	//pos node
	if( strcmp( DATA->train_pos_node[i], msg->train_pos_node ) ) {
		safestrcpy( DATA->train_pos_node[i], msg->train_pos_node, MAX_NODE_CHARACTERS + 1 );
		DATA->updated[i][2] = 1;
	}

	//pos offset
	if( DATA->train_pos_off[i] != msg->train_pos_off ) {
		DATA->train_pos_off[i] = msg->train_pos_off;
		DATA->updated[i][3] = 1;
	}

	//error
	if( DATA->train_err[i] != msg->train_err ) {
		DATA->train_err[i] = msg->train_err;
		DATA->updated[i][4] = 1;
	}

	//max error
	if( DATA->train_max_err[i] != msg->train_max_err ) {
		DATA->train_max_err[i] = msg->train_max_err;
		DATA->updated[i][5] = 1;
	}

	//dest node
	if( strcmp( DATA->train_dest_node[i], msg->train_dest_node ) ) {
		safestrcpy( DATA->train_dest_node[i], msg->train_dest_node, MAX_NODE_CHARACTERS + 1 );
		DATA->updated[i][6] = 1;
	}

	//dest offset
	if( DATA->train_dest_off[i] != msg->train_dest_off ) {
		DATA->train_dest_off[i] = msg->train_dest_off;
		DATA->updated[i][7] = 1;
	}

	//state
	if( strcmp( DATA->train_state[i], msg->train_state ) ) {
		safestrcpy( DATA->train_state[i], msg->train_state, MAX_STATE_CHARACTERS + 1 );
		DATA->updated[i][8] = 1;
	}
}


void draw_entry( struct train_display_data* DATA, int i ) {
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
		ap_printf( &pbuff, "\x1B[%d;29f  \x1B[%d;29f%d", row, row, DATA->train_speeds[i] );
		DATA->updated[i][1] = 0;
	}

	//pos node
	if( DATA->updated[i][2] ) {
		ap_printf( &pbuff, "\x1B[%d;33f     \x1B[%d;33f%s", row, row, DATA->train_pos_node[i] );
		DATA->updated[i][2] = 0;
	}

	//pos offset
	if( DATA->updated[i][3] ) {
		if( DATA->train_pos_off[i] > -9999 && DATA->train_pos_off[i] < 99999 ) {
			ap_printf( &pbuff, "\x1B[%d;40f     \x1B[%d;40f%d", row, row, DATA->train_pos_off[i] );
		} else {
			ap_printf( &pbuff, "\x1B[%d;40fOVER ", row );
		}
		DATA->updated[i][3] = 0;
	}

	//error
	if( DATA->updated[i][4] ) {
		if( DATA->train_err[i] > -999 && DATA->train_err[i] < 9999 ) {
			ap_printf( &pbuff, "\x1B[%d;47f    \x1B[%d;47f%d", row, row, DATA->train_err[i] );
		} else {
			ap_printf( &pbuff, "\x1B[%d;47fOVER", row );
		}
		DATA->updated[i][4] = 0;
	}

	//max error
	if( DATA->updated[i][5] ) {
		if( DATA->train_max_err[i] > -9999 && DATA->train_max_err[i] < 99999 ) {
			ap_printf( &pbuff, "\x1B[%d;53f     \x1B[%d;53f%d", row, row, DATA->train_max_err[i] );
		} else {
			ap_printf( &pbuff, "\x1B[%d;53fOVER ", row );
		}
		DATA->updated[i][5] = 0;
	}

	//dest node
	if( DATA->updated[i][6] ) {
		ap_printf( &pbuff, "\x1B[%d;60f     \x1B[%d;60f%s", row, row, DATA->train_dest_node[i] );
		DATA->updated[i][6] = 0;
	}

	//dest offset
	if( DATA->updated[i][7] ) {
		if( DATA->train_dest_off[i] > -999 && DATA->train_dest_off[i] < 9999 ) {
			ap_printf( &pbuff, "\x1B[%d;67f    \x1B[%d;67f%d", row, row, DATA->train_dest_off[i] );
		} else {
			ap_printf( &pbuff, "\x1B[%d;67fOVER", row );
		}
		DATA->updated[i][7] = 0;
	}

	// state
	if( DATA->updated[i][8] ) {
		ap_printf( &pbuff, "\x1B[%d;73f     \x1B[%d;73f%s", row, row, DATA->train_state[i] );
		DATA->updated[i][8] = 0;
	}

	//reload cursor position, and enable it
	ap_putstr( &pbuff, "\x1B[u\x1B[?25h" );

	//send the print
	Putbuff( COM2, &pbuff );

}


int get_entry_num( struct train_display_data* DATA, int train_num ) {
	int i = 0;
	for( i = 0; i < MAX_TRAINS_DISPLAY; i++ ) {
		if( DATA->train_nums[i] == train_num || DATA->train_nums[i] == -1 ) {
			DATA->train_nums[i] = train_num;
			DATA->updated[i][0] = 1;
			return i;
		}
	}
	return -1;
}


void draw_tab_outline( ) {
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

