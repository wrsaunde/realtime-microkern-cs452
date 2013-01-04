#include <lib/all.h>
#include <config/user.h>

void initialize_targets( struct game_target_graph* TARG_GRAPH ){

	//target a
	TARG_GRAPH->targets[0].node_index = 0;
	TARG_GRAPH->targets[0].track_position.node = 89;	//MR5
	TARG_GRAPH->targets[0].track_position.offset = 100;
	TARG_GRAPH->targets[0].screen_row = 1;
	TARG_GRAPH->targets[0].screen_col = 24;
	TARG_GRAPH->targets[0].edge_up = 15;	//p
	TARG_GRAPH->targets[0].edge_down = 2;	//c
	TARG_GRAPH->targets[0].edge_left = 1;	//b
	TARG_GRAPH->targets[0].edge_right = 3;	//d

	//target b
	TARG_GRAPH->targets[1].node_index = 1;
	TARG_GRAPH->targets[1].track_position.node = 74;	//E11
	TARG_GRAPH->targets[1].track_position.offset = 150;
	TARG_GRAPH->targets[1].screen_row = 2;
	TARG_GRAPH->targets[1].screen_col = 13;
	TARG_GRAPH->targets[1].edge_up = 0;		//a
	TARG_GRAPH->targets[1].edge_down = 4;	//e
	TARG_GRAPH->targets[1].edge_left = 8;	//i
	TARG_GRAPH->targets[1].edge_right = 2;	//c

	//target c
	TARG_GRAPH->targets[2].node_index = 2;
	TARG_GRAPH->targets[2].track_position.node = 58;	//D11
	TARG_GRAPH->targets[2].track_position.offset = 200;
	TARG_GRAPH->targets[2].screen_row = 2;
	TARG_GRAPH->targets[2].screen_col = 24;
	TARG_GRAPH->targets[2].edge_up = 0;		//a
	TARG_GRAPH->targets[2].edge_down = 5;	//f
	TARG_GRAPH->targets[2].edge_left = 1;	//b
	TARG_GRAPH->targets[2].edge_right = 3;	//d

	//target d
	TARG_GRAPH->targets[3].node_index = 3;
	TARG_GRAPH->targets[3].track_position.node = 37;	//C6
	TARG_GRAPH->targets[3].track_position.offset = 150;
	TARG_GRAPH->targets[3].screen_row = 2;
	TARG_GRAPH->targets[3].screen_col = 35;
	TARG_GRAPH->targets[3].edge_up = 0;		//a
	TARG_GRAPH->targets[3].edge_down = 6;	//g
	TARG_GRAPH->targets[3].edge_left = 2;	//c
	TARG_GRAPH->targets[3].edge_right = 7;	//h

	//target e
	TARG_GRAPH->targets[4].node_index = 4;
	TARG_GRAPH->targets[4].track_position.node = 77;	//E14
	TARG_GRAPH->targets[4].track_position.offset = 110;
	TARG_GRAPH->targets[4].screen_row = 3;
	TARG_GRAPH->targets[4].screen_col = 13;
	TARG_GRAPH->targets[4].edge_up = 1;		//b
	TARG_GRAPH->targets[4].edge_down = 8;	//i
	TARG_GRAPH->targets[4].edge_left = 8;	//i
	TARG_GRAPH->targets[4].edge_right = 5;	//f

	//target f
	TARG_GRAPH->targets[5].node_index = 5;
	TARG_GRAPH->targets[5].track_position.node = 16;	//B1
	TARG_GRAPH->targets[5].track_position.offset = 200;
	TARG_GRAPH->targets[5].screen_row = 3;
	TARG_GRAPH->targets[5].screen_col = 24;
	TARG_GRAPH->targets[5].edge_up = 2;		//c
	TARG_GRAPH->targets[5].edge_down = 9;	//j
	TARG_GRAPH->targets[5].edge_left = 4;	//e
	TARG_GRAPH->targets[5].edge_right = 6;	//g

	//target g
	TARG_GRAPH->targets[6].node_index = 6;
	TARG_GRAPH->targets[6].track_position.node = 40;	//C9
	TARG_GRAPH->targets[6].track_position.offset = 0;
	TARG_GRAPH->targets[6].screen_row = 3;
	TARG_GRAPH->targets[6].screen_col = 35;
	TARG_GRAPH->targets[6].edge_up = 3;		//d
	TARG_GRAPH->targets[6].edge_down = 9;	//j
	TARG_GRAPH->targets[6].edge_left = 5;	//f
	TARG_GRAPH->targets[6].edge_right = 7;	//h

	//target h
	TARG_GRAPH->targets[7].node_index = 7;
	TARG_GRAPH->targets[7].track_position.node = 6;	//A7
	TARG_GRAPH->targets[7].track_position.offset = 0;
	TARG_GRAPH->targets[7].screen_row = 3;
	TARG_GRAPH->targets[7].screen_col = 45;
	TARG_GRAPH->targets[7].edge_up = 3;		//d
	TARG_GRAPH->targets[7].edge_down = 10;	//k
	TARG_GRAPH->targets[7].edge_left = 6;	//g
	TARG_GRAPH->targets[7].edge_right = 9;	//i

	//target i
	TARG_GRAPH->targets[8].node_index = 8;
	TARG_GRAPH->targets[8].track_position.node = 95;	//MR8
	TARG_GRAPH->targets[8].track_position.offset = 120;
	TARG_GRAPH->targets[8].screen_row = 6;
	TARG_GRAPH->targets[8].screen_col = 2;
	TARG_GRAPH->targets[8].edge_up = 4;		//e
	TARG_GRAPH->targets[8].edge_down = 11;	//l
	TARG_GRAPH->targets[8].edge_left = 10;	//k
	TARG_GRAPH->targets[8].edge_right = 9;	//j

	//target j
	TARG_GRAPH->targets[9].node_index = 9;
	TARG_GRAPH->targets[9].track_position.node = 123;	//MR156
	TARG_GRAPH->targets[9].track_position.offset = 25;
	TARG_GRAPH->targets[9].screen_row = 6;
	TARG_GRAPH->targets[9].screen_col = 24;
	TARG_GRAPH->targets[9].edge_up = 5;		//f
	TARG_GRAPH->targets[9].edge_down = 12;	//m
	TARG_GRAPH->targets[9].edge_left = 8;	//i
	TARG_GRAPH->targets[9].edge_right = 10;	//k

	//target k
	TARG_GRAPH->targets[10].node_index = 10;
	TARG_GRAPH->targets[10].track_position.node = 30;	//B15
	TARG_GRAPH->targets[10].track_position.offset = 250;
	TARG_GRAPH->targets[10].screen_row = 6;
	TARG_GRAPH->targets[10].screen_col = 45;
	TARG_GRAPH->targets[10].edge_up = 7;	//h
	TARG_GRAPH->targets[10].edge_down = 14;	//o
	TARG_GRAPH->targets[10].edge_left = 9;	//j
	TARG_GRAPH->targets[10].edge_right = 8;	//i

	//target l
	TARG_GRAPH->targets[11].node_index = 11;
	TARG_GRAPH->targets[11].track_position.node = 68;	//E5
	TARG_GRAPH->targets[11].track_position.offset = 150;
	TARG_GRAPH->targets[11].screen_row = 9;
	TARG_GRAPH->targets[11].screen_col = 13;
	TARG_GRAPH->targets[11].edge_up = 8;	//i
	TARG_GRAPH->targets[11].edge_down = 15;	//p
	TARG_GRAPH->targets[11].edge_left = 14;	//0
	TARG_GRAPH->targets[11].edge_right = 12;//m

	//target m
	TARG_GRAPH->targets[12].node_index = 12;
	TARG_GRAPH->targets[12].track_position.node = 51;	//D4
	TARG_GRAPH->targets[12].track_position.offset = 200;
	TARG_GRAPH->targets[12].screen_row = 9;
	TARG_GRAPH->targets[12].screen_col = 24;
	TARG_GRAPH->targets[12].edge_up = 9;	//j
	TARG_GRAPH->targets[12].edge_down = 15;	//p
	TARG_GRAPH->targets[12].edge_left = 11;	//l
	TARG_GRAPH->targets[12].edge_right = 13;//n

	//target n
	TARG_GRAPH->targets[13].node_index = 13;
	TARG_GRAPH->targets[13].track_position.node = 42;	//C11
	TARG_GRAPH->targets[13].track_position.offset = 0;
	TARG_GRAPH->targets[13].screen_row = 9;
	TARG_GRAPH->targets[13].screen_col = 35;
	TARG_GRAPH->targets[13].edge_up = 9;	//j
	TARG_GRAPH->targets[13].edge_down = 16;	//q
	TARG_GRAPH->targets[13].edge_left = 12;	//m
	TARG_GRAPH->targets[13].edge_right = 14;//o

	//target o
	TARG_GRAPH->targets[14].node_index = 14;
	TARG_GRAPH->targets[14].track_position.node = 12;	//A13
	TARG_GRAPH->targets[14].track_position.offset = 0;
	TARG_GRAPH->targets[14].screen_row = 9;
	TARG_GRAPH->targets[14].screen_col = 45;
	TARG_GRAPH->targets[14].edge_up = 10;	//k
	TARG_GRAPH->targets[14].edge_down = 16;	//q
	TARG_GRAPH->targets[14].edge_left = 13;	//n
	TARG_GRAPH->targets[14].edge_right = 11;//l

	//target p
	TARG_GRAPH->targets[15].node_index = 15;
	TARG_GRAPH->targets[15].track_position.node = 70;	//E7
	TARG_GRAPH->targets[15].track_position.offset = 0;
	TARG_GRAPH->targets[15].screen_row = 10;
	TARG_GRAPH->targets[15].screen_col = 13;
	TARG_GRAPH->targets[15].edge_up = 11;	//l
	TARG_GRAPH->targets[15].edge_down = 0;	//a
	TARG_GRAPH->targets[15].edge_left = 9;	//i
	TARG_GRAPH->targets[15].edge_right = 16;//q

	//target q
	TARG_GRAPH->targets[16].node_index = 16;
	TARG_GRAPH->targets[16].track_position.node = 71;	//E8
	TARG_GRAPH->targets[16].track_position.offset = 590;
	TARG_GRAPH->targets[16].screen_row = 10;
	TARG_GRAPH->targets[16].screen_col = 35;
	TARG_GRAPH->targets[16].edge_up = 13;	//n
	TARG_GRAPH->targets[16].edge_down = 0;	//a
	TARG_GRAPH->targets[16].edge_left = 15;	//p
	TARG_GRAPH->targets[16].edge_right = 14;//o

	/*
		a 0
		b 1
		c 2
		d 3
		e 4
		f 5
		g 6
		h 7
		i 8
		j 9
		k 10
		l 11
		m 12
		n 13
		o 14
		p 15
		q 16
	 */


	//initialize the states
	int i = 0;
	for(i = 0; i < GAME_NUM_TARGETS; i++){
		TARG_GRAPH->targets[i].state = TARG_STATE_NORM;
	}

}