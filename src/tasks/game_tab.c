#include <userspace.h>

#define DELAY_TORPEDO_REFRESH 10


struct game_tab_data {
	//do we show this tab
	int tab_show;

	//the information about the targets
	struct game_target_graph targ_graph;
	struct game_target_graph* TARG_GRAPH;

	//which target do we have selected
	int selection;

	//how many rockets we have
	int pro_torps;

	//how many targets are currently destroyed
	int targets_destroyed;

	//won game?
	int won;


};

//helper functions
void draw_ship_outline( struct game_tab_data* DATA );
void initialize_game_tab_data( struct game_tab_data* DATA );
void redraw_all_targets( struct game_tab_data* DATA, struct print_buffer* pbuff );
void draw_target( struct game_tab_data* DATA, struct print_buffer* pbuff, int id );
void update_target( struct game_tab_data* DATA, int id, int state );
void move_cursor( struct game_tab_data* DATA, int direction );
//end of helpers


void task_game_tab( ) {
	struct game_tab_data data;
	struct game_tab_data* DATA = &data;

	Delay( 50 );

	//initialize the display data
	initialize_game_tab_data( DATA );

	//msgs
	struct two_number_message msg;
	struct number_message rpl;
	int recv_tid;

	//register as the dispatcher
	RegisterAs( "gameTAB" );

	CommandOutput( "gameTAB tid: %d", MyTid( ) );

	//register as a tab
	TabRegister( "SPACE" );

	DATA->won = FALSE;

	while( 1 ) {
		//get another message, we have reached the end
		Receive( &recv_tid, (char *)&msg, sizeof (msg) );

		//check message type and train tid
		switch( msg.message_type ) {
			case GAME_NEW:
				Reply( recv_tid, (char *)0, 0 );
				struct two_number_message snd_msg;
				snd_msg.message_type = GAME_WON_MESSAGE;
				CourierSend(WhoIs("gameAI"), (char*)&snd_msg, sizeof(snd_msg));
				initialize_game_tab_data( DATA );
				DATA->won = FALSE;
				Printf( COM2, "\x1B[?25l\x1B[5;61f                  \x1B[u\x1B[?25h");
				break;
			case GAME_MOVE_UP:
			case GAME_MOVE_DOWN:
			case GAME_MOVE_LEFT:
			case GAME_MOVE_RIGHT:
				Reply( recv_tid, (char *)0, 0 );
				move_cursor( DATA, msg.message_type );
				break;
			case GAME_FIRE:
				Reply( recv_tid, (char *)0, 0 );
				if(DATA->won){
					break;
				}
				if( DATA->TARG_GRAPH->targets[DATA->selection].state != TARG_STATE_NORM || DATA->pro_torps == 0 ) {
					//we can't fire here, or no pro torps, just ignore
					break;
				}
				DATA->pro_torps--;
				DATA->targets_destroyed++;

				if(DATA->targets_destroyed > GAME_WIN_NUMBER){
					//you have won
					struct two_number_message send_msg;
					send_msg.message_type = GAME_WON_MESSAGE;
					CourierSend(WhoIs("gameAI"), (char*)&send_msg, sizeof(send_msg));
					DATA->won = TRUE;
					Printf( COM2, "\x1B[?25l\x1B[5;61fYOU WON! n TO PLAY\x1B[u\x1B[?25h");
					break;
				}

				WorkerDelayUntilWithId( Time( ) + GAME_PRO_TORP_REFRESH_TIME, DELAY_TORPEDO_REFRESH );
				//update the display
				if( DATA->tab_show ) {
					Printf( COM2, "\x1B[?25l\x1B[5;30f   \x1B[5;30f%d\x1B[5;52f  \x1B[5;52f%d\x1B[u\x1B[?25h", DATA->pro_torps, DATA->targets_destroyed );
				}
				update_target( DATA, DATA->selection, TARG_STATE_HIT );

				//TODO: SEND A MESSAGE TO THE GAME AI SO IT WILL REPAIR
				struct two_number_message send_msg;
				send_msg.message_type = GAME_DESTROYED_MESSAGE;
				send_msg.num1 = DATA->selection;
				CourierSend(WhoIs("gameAI"), (char*)&send_msg, sizeof(send_msg));
				break;
			case DELAY_WORKER_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				switch( msg.num1 ) {
					case DELAY_TORPEDO_REFRESH:
						DATA->pro_torps++;
						if(DATA->tab_show){
							Printf( COM2, "\x1B[?25l\x1B[5;30f   \x1B[5;30f%d\x1B[u\x1B[?25h", DATA->pro_torps );
						}
						break;
				}
				break;
			case GAME_REPAIRING_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				//set the node to be repairing
				update_target( DATA, msg.num1, TARG_STATE_REPAIR );
				break;
			case GAME_REPAIRING_FAILED_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				//set the node to be hit again, dont update the count though
				update_target( DATA, msg.num1, TARG_STATE_HIT );
				break;
			case GAME_REPAIRED_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				//update the number of hit nodes
				DATA->targets_destroyed--;
				//set the node to be repaired
				update_target( DATA, msg.num1, TARG_STATE_NORM );
				Printf( COM2, "\x1B[?25l\x1B[5;52f  \x1B[5;52f%d\x1B[u\x1B[?25h", DATA->targets_destroyed );
				break;
			case TAB_DISABLE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				DATA->tab_show = FALSE;
				break;
			case TAB_ENABLE_MESSAGE:
				Reply( recv_tid, (char *)0, 0 );
				DATA->tab_show = TRUE;
				clear_tab_space( );
				draw_ship_outline( DATA );
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


void initialize_game_tab_data( struct game_tab_data* DATA ) {

	//initially tab is not showing
	DATA->tab_show = FALSE;

	//initialize the target graph
	DATA->TARG_GRAPH = &DATA->targ_graph;
	initialize_targets( DATA->TARG_GRAPH );

	//set our initial target
	DATA->selection = 0;

	//initialize our amunition
	DATA->pro_torps = GAME_NUM_PRO_TORPS;

	//targets destroyed should be 0
	DATA->targets_destroyed = 0;


}


void draw_ship_outline( struct game_tab_data* DATA ) {
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	/* draw the outlines */
	ap_putstr( &pbuff, "\x1B[?25l\x1B[1;46;37m" ); //turn cursor off andset attributes

	ap_putstr( &pbuff, "\x1B[6;24f                 ------ -------                        " );
	ap_putstr( &pbuff, "\x1B[7;24f        --/- -----/---- -----\\---- -----\\  \\  /|||~~   " );
	ap_putstr( &pbuff, "\x1B[8;24f    /--/ --/ ---------- ---------- ----  \\   |||||~~~  " );
	ap_putstr( &pbuff, "\x1B[9;24f   / ---/   ___     \\     /    ___     \\--\\   \\|||~~   " );
	ap_putstr( &pbuff, "\x1B[10;24f  /=/     _/ - \\_    \\   /   _/ - \\_       \\           " );
	ap_putstr( &pbuff, "\x1B[11;24f       o====== '_|          |_' ======o                " );
	ap_putstr( &pbuff, "\x1B[12;24f  \\=\\      \\_-_/     /   \\    \\_-_/        /           " );
	ap_putstr( &pbuff, "\x1B[13;24f   \\ ---\\           /     \\            /--/   /|||~~   " );
	ap_putstr( &pbuff, "\x1B[14;24f    \\--\\ --\\ ---------- ---------- ----  /   |||||~~~  " );
	ap_putstr( &pbuff, "\x1B[15;24f        --\\- --------------------- --------/  \\|||~~   " );
	ap_putstr( &pbuff, "\x1B[16;24f                 --------------                        " );

	//make the engine thrust red
	ap_putstr( &pbuff, "\x1B[1;46;31m" );
	ap_putstr( &pbuff, "\x1B[7;74f~~" );
	ap_putstr( &pbuff, "\x1B[8;74f~~~" );
	ap_putstr( &pbuff, "\x1B[9;74f~~" );
	ap_putstr( &pbuff, "\x1B[13;74f~~" );
	ap_putstr( &pbuff, "\x1B[14;74f~~~" );
	ap_putstr( &pbuff, "\x1B[15;74f~~" );

	/*
	ap_putstr( &pbuff, "\x1B[6;41f------a-------" );
	ap_putstr( &pbuff, "\x1B[7;32f--/-b-----/----c-----\\----d-----\\  \\  /|||~~" );
	ap_putstr( &pbuff, "\x1B[8;28f/--/ --/e----------f----------g----  \\  h|||||~~~" );
	ap_putstr( &pbuff, "\x1B[9;27f/ ---/   ___     \\     /    ___     \\--\\   \\|||~~" );
	ap_putstr( &pbuff, "\x1B[10;26f/=/     _/ - \\_    \\   /   _/ - \\_       \\" );
	ap_putstr( &pbuff, "\x1B[11;25fi     o====== '_|     j    |_' ======o     k" );
	ap_putstr( &pbuff, "\x1B[12;26f\\=\\      \\_-_/     /   \\    \\_-_/        /" );
	ap_putstr( &pbuff, "\x1B[13;27f\\ ---\\           /     \\            /--/   /|||~~" );
	ap_putstr( &pbuff, "\x1B[14;28f\\--\\ --\\l----------m----------n----  /  o|||||~~~" );
	ap_putstr( &pbuff, "\x1B[15;32f--\\-p---------------------q--------/  \\|||~~" );
	ap_putstr( &pbuff, "\x1B[16;41f--------------" );
	 */


	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" ); //clear attributes and restore cursor
	Putbuff( COM2, &pbuff );

	//DRAW THE TARGETS
	//clear the buffer
	ap_init_buff( &pbuff );
	ap_putstr( &pbuff, "\x1B[?25l" ); //turn cursor off
	redraw_all_targets( DATA, &pbuff ); //draw the targets
	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" ); //clear attributes and restore cursor

	Putbuff( COM2, &pbuff );

	//DRAW THE GAME STATS
	ap_init_buff( &pbuff );
	ap_putstr( &pbuff, "\x1B[?25l" ); //turn cursor off
	ap_printf( &pbuff, "\x1B[5;25fAMMO:%d", DATA->pro_torps );
	ap_printf( &pbuff, "\x1B[5;34fTARGETS DESTROYED:%d", DATA->targets_destroyed );
	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" ); //clear attributes and restore cursor
	Putbuff( COM2, &pbuff );
}


void move_cursor( struct game_tab_data* DATA, int direction ) {
	int old_selection = DATA->selection;
	switch( direction ) {
		case GAME_MOVE_UP:
			DATA->selection = DATA->TARG_GRAPH->targets[old_selection].edge_up;
			break;
		case GAME_MOVE_DOWN:
			DATA->selection = DATA->TARG_GRAPH->targets[old_selection].edge_down;
			break;
		case GAME_MOVE_LEFT:
			DATA->selection = DATA->TARG_GRAPH->targets[old_selection].edge_left;
			break;
		case GAME_MOVE_RIGHT:
			DATA->selection = DATA->TARG_GRAPH->targets[old_selection].edge_right;
			break;
	}

	if( !DATA->tab_show ) {
		return;
	}


	//readraw the change of targets (move cursor)
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	ap_putstr( &pbuff, "\x1B[?25l" );
	draw_target( DATA, &pbuff, old_selection );
	draw_target( DATA, &pbuff, DATA->selection );
	ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" );

	Putbuff( COM2, &pbuff );
}


void redraw_all_targets( struct game_tab_data* DATA, struct print_buffer* pbuff ) {
	if( !DATA->tab_show ) {
		return;
	}
	int i = 0;
	for( i = 0; i < GAME_NUM_TARGETS; i++ ) {
		draw_target( DATA, pbuff, i );
	}
}


void update_target( struct game_tab_data* DATA, int id, int state ) {
	int old_state = DATA->TARG_GRAPH->targets[id].state;
	DATA->TARG_GRAPH->targets[id].state = state;
	if( state != old_state && DATA->tab_show) {
		struct print_buffer pbuff;
		ap_init_buff( &pbuff );
		ap_putstr( &pbuff, "\x1B[?25l" );
		draw_target( DATA, &pbuff, id );
		ap_putstr( &pbuff, "\x1B[0m\x1B[u\x1B[?25h" );
		Putbuff( COM2, &pbuff );
	}
}


void draw_target( struct game_tab_data* DATA, struct print_buffer* pbuff, int id ) {
	if( !DATA->tab_show ) {
		return;
	}

	if( id < 0 || id >= GAME_NUM_TARGETS ) {
		return;
	}

	int fgcolour = 0;
	int bgcolour = 0;
	char xhair = 'X';

	if( DATA->selection == id ) {
		bgcolour = 40;
		xhair = 'X';
	} else {
		bgcolour = 46;
		xhair = 'X';
	}

	//set the background
	switch( DATA->TARG_GRAPH->targets[id].state ) {
		case TARG_STATE_NORM:
			fgcolour = 32;
			break;
		case TARG_STATE_HIT:
			fgcolour = 31;
			break;
		case TARG_STATE_REPAIR:
			fgcolour = 33;
			break;
		default:
			fgcolour = 32;
			break;
	}

	//move the cursor to the position and print the xhair character
	ap_printf( pbuff, "\x1B[%d;%d;1m\x1B[%d;%df%c\x1B[0m", fgcolour, bgcolour, DATA->TARG_GRAPH->targets[id].screen_row + 5, DATA->TARG_GRAPH->targets[id].screen_col + 23, xhair );
}

