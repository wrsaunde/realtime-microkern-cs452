#include <userspace.h>

void task_system_switch_server() {
	int recv_tid = 0, i = 0; 
	int sw = 0, setting = 0;
	int switch_states[CONFIG_NUM_SWITCHES];
	int tid;
	struct number_message rpl;
	struct two_number_message msg;

	RegisterAs("switchserv");
	
	for(i = 0; i < CONFIG_NUM_SWITCHES; i++) {
		switch_states[i] = SWITCH_UNKNOWN;
	}
	
	while(1) {
		//get another message, we have reached the end
		Receive(&recv_tid, (char *) &msg, sizeof(msg));
		
		//act on the message type
		switch(msg.message_type) {
			case SWITCH_CTRL_MESSAGE:
				//reply immediately
				sw = msg.num1;
				setting = msg.num2;
				
				rpl.message_type = ACK_MESSAGE;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				
				//Send command on to track server
				tid = WhoIs("mdiserv");

				//check the return value of WhoIs
				assert(tid >= 0, "tid >= 0")
				
				msg.message_type = SWITCH_CTRL_MESSAGE;
				msg.num1 = sw;
				msg.num2 = setting;

				CourierSend(tid, (char *) &msg, sizeof(msg));
				
				switch_states[switch_index(sw)] = setting;
				if(setting == SWITCH_STRAIGHT) {
                                        DisplaySwitch( sw, 'S');					
				} else if(setting == SWITCH_CURVED) {
                                        DisplaySwitch( sw, 'C');					
				}
				break;
			case SWITCH_STATUS_MESSAGE:
				//reply immediately
				rpl.message_type = ACK_MESSAGE;
				rpl.num = switch_states[switch_index(msg.num1)];
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				break;
			default:
				//unexpected message, reply and say so
				rpl.message_type = INVALID_OPERATION_MESSAGE;
				rpl.num = 0;
				Reply(recv_tid, (char *) &rpl, sizeof(rpl));
				break;
		}
		
	}
}

