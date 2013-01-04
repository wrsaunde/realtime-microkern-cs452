#include <userspace.h>

void task_user_draw_ui() {
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );
	
	//Clear the screen
	ap_putstr( &pbuff, "\x1B[2J");
	
	/* draw the cli prompt */
	ap_putstr( &pbuff, "\x1B[31m");	//set attributes
	
	ap_putstr( &pbuff, "\x1B[24;1f");	//move to prompt
	ap_putstr( &pbuff, "root@UnionStation# ");
	
	//save the cursor location
	ap_putstr( &pbuff, "\x1B[s");
	
	ap_putstr( &pbuff, "\x1B[0m");	//clear attributes
	/* end of titles */
	
	
	
	/* Set the scrolling section */
	ap_putstr( &pbuff, "\x1B[18;23r");
	/* End of set scrolling */
	
	
	
	/* Write the titles */
	ap_putstr( &pbuff, "\x1B[1;32m");	//set attributes
	
	ap_putstr( &pbuff, "\x1B[1;17f");	//move to title place
	ap_putstr( &pbuff, "Train App | Group CS452_06 | Time ");
	
	ap_putstr( &pbuff, "\x1B[2;8f");	//move to title place
	ap_putstr( &pbuff, "SWITCHES");
	
	ap_putstr( &pbuff, "\x1B[9;5f");	//move to title place
	ap_putstr( &pbuff, "RECENT SENSORS");
	
	ap_putstr( &pbuff, "\x1B[17;31f");	//move to title place
	ap_putstr( &pbuff, "COMMAND OUTPUT");
	
	ap_putstr( &pbuff, "\x1B[0m");	//clear attributes
	/* end of titles */
	
	/* Write the switches */
	ap_putstr( &pbuff, "\x1B[0m");	//set attributes
	
	//1st col
	ap_putstr( &pbuff, "\x1B[3;3f1:");
	ap_putstr( &pbuff, "\x1B[4;3f2:");
	ap_putstr( &pbuff, "\x1B[5;3f3:");
	ap_putstr( &pbuff, "\x1B[6;3f4:");
	ap_putstr( &pbuff, "\x1B[7;3f5:");
	ap_putstr( &pbuff, "\x1B[8;3f6:");
	
	//2nd col
	ap_putstr( &pbuff, "\x1B[3;8f7:");
	ap_putstr( &pbuff, "\x1B[4;8f8:");
	ap_putstr( &pbuff, "\x1B[5;8f9:");
	ap_putstr( &pbuff, "\x1B[6;7f10:");
	ap_putstr( &pbuff, "\x1B[7;7f11:");
	ap_putstr( &pbuff, "\x1B[8;7f12:");
	
	//3rd col
	ap_putstr( &pbuff, "\x1B[3;12f13:");
	ap_putstr( &pbuff, "\x1B[4;12f14:");
	ap_putstr( &pbuff, "\x1B[5;12f15:");
	ap_putstr( &pbuff, "\x1B[6;12f16:");
	ap_putstr( &pbuff, "\x1B[7;12f17:");
	ap_putstr( &pbuff, "\x1B[8;12f18:");
	
	//4th col
	ap_putstr( &pbuff, "\x1B[3;17f153:");
	ap_putstr( &pbuff, "\x1B[4;17f154:");
	ap_putstr( &pbuff, "\x1B[5;17f155:");
	ap_putstr( &pbuff, "\x1B[6;17f156:");
	
	ap_putstr( &pbuff, "\x1B[0m");	//clear attributes*/
	/* end of switches */

	//push the buffer
	Putbuff( COM2, &pbuff );
	ap_init_buff( &pbuff );
	
	
	/* Draw the borders */
	ap_putstr( &pbuff, "\x1B[2;33m");	//set attributes
	
	//print each line
	ap_putstr( &pbuff, "\x1B[2;1f");	//move to location
	ap_putstr( &pbuff, "######\x1B[2;17f###############################################################");	//write border
	
	int i = 0;
	for(i = 3; i <= 16; i++){
		ap_printf( &pbuff, "\x1B[%d;1f",i);	//move to location
		ap_printf( &pbuff, "#\x1B[%d;23f#\x1B[%d;79f#",i,i);	//write border
	}
	
	ap_putstr( &pbuff, "\x1B[3;1f");	//move to location
	ap_putstr( &pbuff, "#\x1B[3;23f#\x1B[3;37f#\x1B[3;51f#\x1B[3;65f#\x1B[3;79f#");	//write border
	
	ap_putstr( &pbuff, "\x1B[4;1f");	//move to location
	ap_putstr( &pbuff, "#\x1B[4;23f#########################################################");	//write border
	
	ap_putstr( &pbuff, "\x1B[9;1f");	//move to location
	ap_putstr( &pbuff, "###\x1B[9;20f####\x1B[9;79f#");	//write border
	
	ap_putstr( &pbuff, "\x1B[17;1f");	//move to location
	ap_putstr( &pbuff, "#############################\x1B[17;46f##################################");	//write border
	
	ap_putstr( &pbuff, "\x1B[0m");	//clear attributes
	
	/* End of borders */
	
	Putbuff( COM2, &pbuff );
	
	//set the switch states
	i = 0;
	for(i = 0; i < 22; i++){
		DisplaySwitch( switch_index_inverse(i), 'U');
	}

	Exit();
}


