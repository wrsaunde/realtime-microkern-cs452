#include <sys_calls/sys_calls.h>

#include <userspace.h>


void TabRegister( char* title ) {
	int tid = -1;
	tid = WhoIs( "tabber" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return;
	}

	//create the message
	struct short_str_message msg, rpl;

	msg.message_type = TAB_REGISTER_MESSAGE;
	safestrcpy( msg.str, title, CONFIG_SHORT_STR_MSG_LENGTH );

	//send the register message
	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof(rpl) );
}


void TabLeft( ) {
	int tid = -1;
	tid = WhoIs( "tabber" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return;
	}

	//create the message
	struct empty_message msg, rpl;

	msg.message_type = TAB_LEFT_MESSAGE;

	//send the register message
	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof(rpl) );
}


void TabRight( ) {
	int tid = -1;
	tid = WhoIs( "tabber" );

	//check the return value of WhoIs
	if( tid < 0 ) {
		return;
	}

	//create the message
	struct empty_message msg, rpl;

	msg.message_type = TAB_RIGHT_MESSAGE;

	//send the register message
	Send( tid, (char *)&msg, sizeof (msg), (char *)&rpl, sizeof(rpl) );
}
