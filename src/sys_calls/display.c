/**
 *
 * display.c
 *
 **/

#include <userspace.h>

void DisplaySensorList( int* recent_list){
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );
	int i = 0;
	
	//hide the cursor
	ap_printf(&pbuff, "\x1B[?25l\x1B[u");
	
	for(i = 0; i < 7; i++){
		if(recent_list[i] >= 0){
			char letter = sensor_letter(recent_list[i]);
			int num = sensor_number(recent_list[i]);
			ap_printf(&pbuff, "\x1B[%d;3f%d: %c%d  ",i + 10, i + 1, letter, num);
		}else{
			ap_printf(&pbuff, "\x1B[%d;3f%d: N/A",i + 10, i + 1);
		}
	}
	//show the cursor and restore position
	ap_printf(&pbuff, "\x1B[u\x1B[?25h");
	Putbuff(COM2, &pbuff);
}

void DisplaySwitch( int switch_num, char c){
	int colour = 0;
	int column = -1;
	int row = -1;
	
	switch(c){
		case 's':
		case 'S':
			colour = 34;	//BLUE
			break;
		case 'c':
		case 'C':
			colour = 36;	//CYAN
			break;
		case 'u':
		case 'U':
			colour = 35;	//MAGENTA
			break;
		default:
			colour = 31;	//RED
			break;
	}
	
	switch(switch_num){
		case 1:
			row = 3;
			column = 5;
			break;
		case 2:
			row = 4;
			column = 5;
			break;
		case 3:
			row = 5;
			column = 5;
			break;
		case 4:
			row = 6;
			column = 5;
			break;
		case 5:
			row = 7;
			column = 5;
			break;
		case 6:
			row = 8;
			column = 5;
			break;
		case 7:
			row = 3;
			column = 10;
			break;
		case 8:
			row = 4;
			column = 10;
			break;
		case 9:
			row = 5;
			column = 10;
			break;
		case 10:
			row = 6;
			column = 10;
			break;
		case 11:
			row = 7;
			column = 10;
			break;
		case 12:
			row = 8;
			column = 10;
			break;
		case 13:
			row = 3;
			column = 15;
			break;
		case 14:
			row = 4;
			column = 15;
			break;
		case 15:
			row = 5;
			column = 15;
			break;
		case 16:
			row = 6;
			column = 15;
			break;
		case 17:
			row = 7;
			column = 15;
			break;
		case 18:
			row = 8;
			column = 15;
			break;
		case 153:
			row = 3;
			column = 21;
			break;
		case 154:
			row = 4;
			column = 21;
			break;
		case 155:
			row = 5;
			column = 21;
			break;
		case 156:
			row = 6;
			column = 21;
			break;
		
	}
	
	if( column != -1 && row != -1 ){
		Printf(COM2, "\x1B[?25l\x1B[%d;%df\x1B[1;%dm%c\x1B[0m\x1B[u\x1B[?25h", row, column, colour, c);
	}
}

