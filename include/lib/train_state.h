#ifndef __LIB_TRAIN_STATE_H__
#define __LIB_TRAIN_STATE_H__

#include <userspace.h>

//delay worker codes
#define DELAY_STOP_WAKEUP 1

//sensor watch codes
#define SOS_SENSOR_WATCH 0
#define UNATTRIBUTED_SENSOR_WATCH 1
#define NORMAL_SENSOR_WATCH 2

//update the state based on the current time, should be called
//on every receive before acting
void refresh_state( struct train_data* DATA );

//update to say we have arrived at destination
void destination_arrive( struct train_data* DATA );

//update the train state due to a sensor hit
void update_state_from_sensor( struct train_data* DATA, int sensor_node, int sensor_time );

void create_unatrib_sensor_watch( struct train_data* DATA );

void create_sensor_watch( struct train_data* DATA );

//set the train to be in an unknown position
void update_state_lost( struct train_data* DATA );

struct position guess_position( struct train_data* DATA );

//get the current stop distance
int current_stop_distance( struct train_data* DATA );

//get the stop distance from a speed
int stop_distance( struct train_data* DATA, struct velocity vi);

//get the greatest stop distance between current, and target
int greatest_stop_distance( struct train_data* DATA );

void set_train_speed( struct train_data* DATA, int speed );

void reverse_train( struct train_data* DATA );

//return the proper jerk
int jerk( struct train_data* DATA, struct velocity vi, struct velocity vf );

/* mdi server msg functions */
void mdi_set_speed( struct train_data* DATA, int speed );

void mdi_reverse( struct train_data* DATA );
/* end of mdi server msg functions */

//notify interested tasks (such as the route server) about our current position
void notify_position( struct train_data * DATA );

//msg the route server that we can release reservations
void notify_release_reservations( struct train_data* DATA );


#endif
