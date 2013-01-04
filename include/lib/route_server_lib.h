/*
 * route_server_lib.h
 */

#ifndef ROUTE_SERVER_LIB_H_
#define ROUTE_SERVER_LIB_H_

typedef enum {
	RSV_NONE,
	RSV_ON_ROUTE,
	RSV_DESTINATION,
	RSV_HELD
} reservation_status;

struct reservation_data {
	reservation_status status[TRACK_MAX][2][CONFIG_MAX_TRAINS];
	int num_trains;
};

struct route_server_train_data {
	int train_number;
	int train_index;
	int train_length;
	int tid;
	struct train_route route;
	struct position last_pos;
	int requested_route_length;
};

struct route_server_data {
	struct route_server_train_data train_data[CONFIG_MAX_TRAINS];
	int track_number;
	int num_trains;
	struct track_node track[TRACK_MAX];
	struct reservation_data reservations;
};

//Notify a train of a change in reservation
void notify_reservation(struct route_server_data *DATA, int train_index, int node, int edge, int length_reserved);

void init_route_server_data(struct route_server_data *DATA);

//Get the index for a given train number
int get_train_index(struct route_server_data *DATA, int train_number);

//Wrapper function for creating a new route
void new_route(struct route_server_data *DATA, int train_number, struct position *pos1, struct position *pos2);

/*
 * Route creation functions
 */

//Add a secondary route at the given node, returns length in number of nodes
int add_secondary_route(struct track_node *track, int primary_index, int expected_direction, struct train_route * route, int desired_length);

//Shortest route from node1 to node2
int shortest_route(struct route_server_data * DATA, int train_number, struct train_route * route,
		struct position * pos1, struct position * pos2, int reverse_cost, int reserved_cost);

/*
 * Reservation functions
 */


int can_reserve(struct route_server_data * DATA, int train_index, int node, int edge);

int reserve_conflict(struct route_server_data * DATA, int train_index, int node, int edge);

int reserve(struct route_server_data * DATA, int train_index, int node, int edge);

void free_reservation(struct route_server_data * DATA, int train_index, int node, int edge);

//Give reservations along a secondary route for the given distance
void give_secondary_reservations(struct route_server_data * DATA, int train_index, int primary_index, int distance);

//Try your best to give reservations to the train at train_index so that it can go at least
//distance before it has to stop.
void give_reservations(struct route_server_data * DATA, int train_index);

//Release reservations along the secondary route starting from the given node
void release_secondary_reservations(struct route_server_data * DATA, int train_index, int primary_index);

//Release reservations up to the given position on the given train's route
void release_reservations(struct route_server_data * DATA, int train_index, struct position * pos);

//Release reservations along the secondary route starting from the given node
void route_release_secondary_reservations(struct train_route * route, int primary_index);

//Release reservations up to the given position on the given train's route
void route_release_reservations(struct train_route * route, struct position * pos);

//Release all reservations for a train
void clear_reservations(struct route_server_data * DATA, int train_index);

//Clear all reservations outside of the train's current route
void clear_extra_reservations(struct route_server_data * DATA, int train_index);

//Update reservation based on a message
void update_reservation(struct train_route *route, int node, int edge, int length_reserved);

//Reserve where the train is now
void reserve_position(struct route_server_data * DATA, int train_index, struct position * pos);

#endif /* ROUTE_SERVER_LIB_H_ */
