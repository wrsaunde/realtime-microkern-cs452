#ifndef __LIB_GAME_H__
#define __LIB_GAME_H__

#include <lib/route.h>

#define GAME_NUM_TARGETS 17

#define TARG_STATE_NORM 0
#define TARG_STATE_HIT 1
#define TARG_STATE_REPAIR 2

struct game_target_node {
	//index number in the graph
	int node_index;

	//where on the track is the target
	struct position track_position;

	//what is the state of the target
	int state;

	//position of the node on the screen
	int screen_row;
	int screen_col;

	//edges to other nodes
	int edge_up;
	int edge_down;
	int edge_left;
	int edge_right;
};

struct game_target_graph {
	struct game_target_node targets[GAME_NUM_TARGETS];
};

void initialize_targets( struct game_target_graph* TARG_GRAPH );


#endif
