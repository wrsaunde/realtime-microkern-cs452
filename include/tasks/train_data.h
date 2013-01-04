#ifndef __TRAIN_DATA_H__
#define __TRAIN_DATA_H__

#include <lib/route.h>
#include <lib/motion.h>
#include <tasks/message.h>

//internal train states
#define STATE_NORMAL 0
#define STATE_STOPPED 1
#define STATE_STOPPING 2
#define STATE_ACCELERATING 3
#define STATE_STOPPING_REVERSE 4
#define STATE_STOPPED_REVERSE 5

//directions
#define DIRECTION_FORWARD 0
#define DIRECTION_BACKWARD 1
#define DIRECTION_UNKNOWN 2

#define NUM_LOGICAL_SPEEDS 30

struct train_calibration {
	int number;
	
	int length; //mm
	int pickup_len;
	int front_len;
	int back_len;

	int jerk_const_up;
	int jerk_const_down;

	int speed_low;
	int speed_med;
	int speed_high;

	int disttotal[NUM_LOGICAL_SPEEDS]; //mm per 100 ticks
	int timetotal[NUM_LOGICAL_SPEEDS];
	int stopdist[NUM_LOGICAL_SPEEDS]; //mm
	int stopdistsafe[NUM_LOGICAL_SPEEDS]; //mm

};

struct sensor_prediction{
	int nodes[CONFIG_MAX_UPCOMING_SENSORS];
	int distances[CONFIG_MAX_UPCOMING_SENSORS];
	int primary[CONFIG_MAX_UPCOMING_SENSORS];
	int num_sensors;
};

struct train_state {
	//the current state identifier
	int current;
	int lost;

	//last concrete position of the train
	struct position last_sensor_pos;
	int last_sensor_time;

	//guessed position of the last state change
	struct position state_change_pos;
	int state_change_start_time;
	int state_change_finish_time;

	//guessed position of the train
	struct position current_pos_guess;

	//direction the train is travelling
	int direction;

	//information about what speed we are travelling at
	int speed_target;
	int speed_last;

	//have we changed speed since last sensor
	int changed_speed;

	//velocity information, this is used instead of speed when in
	//accelerating mode
	struct velocity velocity_target;
	struct velocity velocity_last;

};

struct train_data {
	int train_number;

	int track_number;
	struct track_node track[TRACK_MAX];

	//details on the state of the train
	struct train_state state;

	//information about the next sensors
	struct sensor_prediction sensors;

	//where is this train routing to
	struct position dest;

	//the route that this train is currently on
	struct train_route route;
	struct train_route route_buffer;
	int on_secondary_route;

	//calibration data
	struct train_calibration calibration;

	int error_last;
	int error_max;

	//important tid information
	int tid_me;
	int tid_mdi;
	int tid_sensor_interp;
	int tid_route_serv;

	//current time (updated on every wakeup)
	int current_time;

	//is the stop working running?
	int stop_worker_running;

	//Last time that we gave an update to various tasks (ie. the route server) that depend on our position
	int last_pos_notify_time;

	//last time we received a reservation update
	int last_res_update_time;
	int waiting_for_res;

	//last time we switched switches
	int last_switch_time;
	int last_lost_check_time;
};

void init_train_calibration( struct train_calibration * trdata, int number );

#endif

