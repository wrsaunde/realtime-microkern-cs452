#include <userspace.h>


void task_game_keyboard( ) {

	char c;


	//control looping of Getc
	int flag_loop = TRUE;

	//messages to trigger the game input
	struct two_number_message msg;
	int recv_tid;

	//become
	RegisterAs( "gameKEY" );
	

	//get the tid of the tab task
	int game_tab_tid = WhoIs( "gameTAB" );
	while(game_tab_tid < 0){
		Delay(10);
		game_tab_tid = WhoIs( "gameTAB" );
	}
	CommandOutput("GOT GAME TAB TID %d", game_tab_tid);

	while( 1 ) {
		flag_loop = TRUE;
		//wait for the cli to trigger us to start taking input
		Receive( &recv_tid, (char*)&msg, sizeof (msg) );

		while( flag_loop ) {
			c = Getc( COM2 );

			struct two_number_message send_msg;

			switch( c ) {
				case 'w':
					send_msg.message_type = GAME_MOVE_UP;
					Send( game_tab_tid, (char*)&send_msg, sizeof (send_msg), (char*)0, 0 );
					break;
				case 'a':
					send_msg.message_type = GAME_MOVE_LEFT;
					Send( game_tab_tid, (char*)&send_msg, sizeof (send_msg), (char*)0, 0 );
					break;
				case 's':
					send_msg.message_type = GAME_MOVE_DOWN;
					Send( game_tab_tid, (char*)&send_msg, sizeof (send_msg), (char*)0, 0 );
					break;
				case 'd':
					send_msg.message_type = GAME_MOVE_RIGHT;
					Send( game_tab_tid, (char*)&send_msg, sizeof (send_msg), (char*)0, 0 );
					break;
				case 'n':
					send_msg.message_type = GAME_NEW;
					Send( game_tab_tid, (char*)&send_msg, sizeof (send_msg), (char*)0, 0 );
					break;
				case CHAR_ENTER:
					send_msg.message_type = GAME_FIRE;
					Send( game_tab_tid, (char*)&send_msg, sizeof (send_msg), (char*)0, 0 );
					break;
				case 'q':
					//switch back to the cli
					flag_loop = FALSE;
					break;
				default:
					break;
			}

		}

		//wake the cli back up
		Reply( recv_tid, (char*)0, 0 );

	}
}


