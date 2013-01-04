#ifndef __LIB_ROUTE_H__
#define __LIB_ROUTE_H__

#include <tasks/track_node.h>
#include <tasks/track_data.h>
#include <config/train_constants.h>
#include <config/user.h>

struct position {
	short node;
	short edge;
	int offset;
};

struct route_node {
	//Graph number of the node
	int node:8;
	//Cumulative distance to the node from node 0
	int distance;
	//0 = not reserved for this train, INT_MAX = fully reserved for this train,
	//any other value = that many mm reserved for this train
	int length_reserved;
	//Index into the secondary route array, corresponding to the secondary path if
	//the switch at this node fails
	int secondary_route_index:8;
	//Index (0  or 1) of the selected edge
	int edge:2;
	//Do I need to reverse after stopping on this section? 1 = yes, 0 = no (usually no)
	int reverse_here:2;
	//Did I pass this node already?
	int passed:2;
};

struct train_route {
	//Primary route - where we want the train to go
	struct route_node primary[CONFIG_MAX_ROUTE_SIZE + 1];
	//Secondary route, with an array entry corresponding to each switch node.
	//Where the train might go if a switch fails.
	struct route_node secondary[CONFIG_MAX_NUM_SECONDARY_ROUTES + 1][CONFIG_MAX_SECONDARY_ROUTE_SIZE];

	//Number of nodes along current route
	int primary_size;
	//Number of nodes along each secondary route
	int secondary_size[CONFIG_MAX_NUM_SECONDARY_ROUTES + 1];

	int num_secondary_routes;

	struct position destination;

};

struct track_reservations {
	int edge_owner[TRACK_MAX][2];
};


//Initialize route structure
void init_route(struct train_route * r);

//Copy a route
void copy_route(volatile struct train_route * source, volatile struct train_route * dest);

/*
 *Distance functions
 */

//Distance from node1 to node2 on the track
int track_distance(struct track_node *track, struct position *pos1, struct position *pos2);

//Index of the node along the primary route
int primary_route_index(struct train_route*route, int start, int node);

//Index of the node along the secondary route
int secondary_route_index(struct train_route * route, int secondary_index, int start, int node);

//Distance between two nodes along the primary route
int primary_route_distance(struct train_route *route, struct position *pos1, struct position *pos2);

//Update position based on how far we've travelled from the last node
int traverse_route(struct track_node *track, struct position * pos, struct train_route * route);

/*
 * Train location functions
 */

//Edge we expect to take going forward from each node on the track
int track_expected_edge(struct track_node * track, int node);

//Given a node and distance in front of it, traverse the track in a forward direction until we
//find where the corresponding position is on the track
void traverse_track(struct track_node * track, struct position * pos);

//Find the position of the front of the train, given a position and it's length in front of it
struct position train_front(struct track_node * track, struct position * pos, int front_length);

//Find the reverse of the current position (node + offset)
struct position reverse_position(struct track_node * track, struct position * pos);

//Find the position of the back of the train, given a position and it's length behind
struct position train_back(struct track_node * track, struct position * pos, int front_length);

int passed_too_many_sensors(struct track_node * track, struct train_route * route, struct position * pos);

/*
 * Switch functions
 */

//Set upcoming switches, from the given position for the given distance
void set_upcoming_switches(struct track_node *track, struct train_route * route, struct position * pos, int max_dist,
		int min_dist, int switch_to_set);


/*
 * Sensor Prediction Functions
 */

//Distance until the next sensor we expect to hit from the current position
int next_primary_sensor_distance(struct track_node *track, struct position * pos, struct train_route * route,
		int * sensor);

//Distance to first secondary sensor along the route
int first_secondary_sensor_distance(struct track_node * track, struct train_route *route, int primary_index, int * sensor);

//Make a prediction for which sensors we could hit next, returns the number of sensors
//Doesn't take into account quantum train effects
int predict_sensors_classical(struct track_node *track, struct position * pos, struct train_route * route, int * sensors,
		int * distances, int * primary);

//Make a prediction for which sensors we could hit next, returns the number of sensors
//Includes quantum train effects
int predict_sensors_quantum(struct track_node *track, struct position * pos, struct train_route * route, int * sensors, int * distances, int * primary, int last_sensor);

//Distance until the next sensor we expect to hit from the current position
int next_sensor_distance(struct track_node *track, struct position * pos, struct route_node * route_nodes,
		int route_length, int * sensor);

/*
 * Stop distance functions
 */

//Find length (in mm) of secondary route
int secondary_route_length(struct track_node * track, struct train_route *route, int primary_index);

//Find guaranteed length (in mm) we can travel along the route
int route_guaranteed_length(struct track_node * track, struct position *pos, struct train_route * route, int last_sensor);

//Offset from current node at which we need to send the stop command
int stop_offset(struct track_node * track, struct position *pos, struct train_route * route, int stop_distance, int last_sensor);

#endif
