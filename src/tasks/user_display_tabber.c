#include <userspace.h>


//max number of trains tab can display
#define MAX_TABS 4
#define MAX_TITLE_CHARACTERS 11

struct tab_data {
	int num_tabs;

	int tab_tids[MAX_TABS];
	int active_tab;
	char tab_titles[MAX_TABS][MAX_TITLE_CHARACTERS + 1];
};

int get_tab_entry_num( struct tab_data* DATA, int tab_tid );
void initialize_tab_data( struct tab_data* DATA );
void draw_active_title( struct tab_data* DATA, int tab_num );
void draw_inactive_title( struct tab_data* DATA, int tab_num );

void disable_tab( struct tab_data* DATA, int tab_num );
void activate_tab( struct tab_data* DATA, int index );
void switch_tab_left( struct tab_data* DATA );
void switch_tab_right( struct tab_data* DATA );


void task_user_display_tabber( ) {
	struct tab_data data;
	struct tab_data* DATA = &data;

	//initialize the display data
	initialize_tab_data( DATA );

	//msgs
	struct short_str_message msg, rpl;
	int recv_tid;

	//register as the dispatcher
	RegisterAs( "tabber" );

	CommandOutput("tabber tid: %d", MyTid());

	while( 1 ) {
		//get another message, we have reached the end
		Receive( &recv_tid, (char *)&msg, sizeof (msg) );

		//check message type and train tid
		switch( msg.message_type ) {
			case TAB_REGISTER_MESSAGE:
				rpl.message_type = ACK_MESSAGE;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );

				//register the tab
				int index = get_tab_entry_num( DATA, recv_tid );
				if( index != -1 ) {
					DATA->num_tabs++;
					DATA->tab_tids[index] = recv_tid;
					safestrcpy( DATA->tab_titles[index], msg.str, MAX_TITLE_CHARACTERS + 1 );

					//write the updated entry
					if( DATA->num_tabs == 1 ) {
						DATA->active_tab = index;
						activate_tab( DATA, index );
					}else{
						disable_tab( DATA, index );
					}
				}
				break;
			case TAB_LEFT_MESSAGE:
				rpl.message_type = ACK_MESSAGE;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );

				switch_tab_left( DATA );
				break;
			case TAB_RIGHT_MESSAGE:
				rpl.message_type = ACK_MESSAGE;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );

				switch_tab_right( DATA );
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				Reply( recv_tid, (char *)&rpl, sizeof (rpl) );
				break;
		}
	}

	Exit( );
}


void switch_tab_left( struct tab_data* DATA ) {
	if( DATA->active_tab == -1 ) {
		return;
	}
	int next_tab = DATA->active_tab - 1 ;
	if( next_tab <= -1 || DATA->tab_tids[next_tab] < 0 ) {
		next_tab = DATA->num_tabs - 1;
	}
	if( DATA->tab_tids[next_tab] < 0 ) {
		next_tab = -1;
	}

	disable_tab( DATA, DATA->active_tab );
	activate_tab( DATA, next_tab );
	DATA->active_tab = next_tab;
}


void switch_tab_right( struct tab_data* DATA ) {
	if( DATA->active_tab == -1 ) {
		return;
	}
	int next_tab = DATA->active_tab + 1;
	if( next_tab >= MAX_TABS || DATA->tab_tids[next_tab] < 0 ) {
		next_tab = 0;
	}
	if( DATA->tab_tids[next_tab] < 0 ) {
		next_tab = -1;
	}

	disable_tab( DATA, DATA->active_tab );
	activate_tab( DATA, next_tab );
	DATA->active_tab = next_tab;
}


void activate_tab( struct tab_data* DATA, int index ) {
	if( DATA->tab_tids[index] < 0 ) {
		return;
	}

	struct empty_message msg;
	msg.message_type = TAB_ENABLE_MESSAGE;

	Send( DATA->tab_tids[index], (char *)&msg, sizeof (msg), (char *)0, 0 );
	draw_active_title( DATA, index);
}


void disable_tab( struct tab_data* DATA, int index ) {
	if( DATA->tab_tids[index] < 0 ) {
		return;
	}

	struct empty_message msg;
	msg.message_type = TAB_DISABLE_MESSAGE;

	Send( DATA->tab_tids[index], (char *)&msg, sizeof (msg), (char *)0, 0 );
	draw_inactive_title( DATA, index );
}


void initialize_tab_data( struct tab_data* DATA ) {

	DATA->num_tabs = 0;
	DATA->active_tab = -1;
	int i = 0;
	for( i = 0; i < MAX_TABS; i++ ) {
		DATA->tab_tids[i] = -1;
		DATA->tab_titles[i][0] = '\0';
	}
}


int get_tab_entry_num( struct tab_data* DATA, int tab_tid ) {
	int i = 0;
	for( i = 0; i < MAX_TABS; i++ ) {
		if( DATA->tab_tids[i] == tab_tid || DATA->tab_tids[i] == -1 ) {
			DATA->tab_tids[i] = tab_tid;
			return i;
		}
	}
	return -1;
}


void draw_active_title( struct tab_data* DATA, int tab_num ) {
	if( tab_num < 0 ) {
		return;
	}

	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	int col = 24 + tab_num * 14;
	//turn off the cursor, set attributes
	ap_putstr( &pbuff, "\x1B[?25l\x1B[34;47;1m" );
	ap_printf( &pbuff, "\x1B[3;%df             ", col, DATA->tab_titles[tab_num] );
	ap_printf( &pbuff, "\x1B[3;%df %s", col, DATA->tab_titles[tab_num] );

	//reset attributes, reload cursor position, and enable it
	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" );

	Putbuff( COM2, &pbuff );
}


void draw_inactive_title( struct tab_data* DATA, int tab_num ) {
	if( tab_num < 0 ) {
		return;
	}

	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	int col = 24 + tab_num * 14;
	//turn off the cursor, set attributes
	ap_putstr( &pbuff, "\x1B[?25l\x1B[35;1m" );
	ap_printf( &pbuff, "\x1B[3;%df             ", col, DATA->tab_titles[tab_num] );
	ap_printf( &pbuff, "\x1B[3;%df %s", col, DATA->tab_titles[tab_num] );

	//reset attributes, reload cursor position, and enable it
	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" );

	Putbuff( COM2, &pbuff );
}

