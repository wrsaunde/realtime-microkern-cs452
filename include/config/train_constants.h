/*
	train_constants.h
	Defines common keywords
*/

#ifndef __TRAIN_CONSTANTS__H__
#define __TRAIN_CONSTANTS__H__

#define TRACK_UNINITIALIZED -1
#define TRACK_A 0
#define TRACK_B 1

#define SWITCH_STRAIGHT 33
#define SWITCH_CURVED 34
#define SWITCH_UNKNOWN 32

#define CONFIG_NUM_TRAINS 99
#define CONFIG_NUM_SENSOR_MODULES 5
#define CONFIG_NUM_SENSORS 80
#define CONFIG_SENSOR_RESULT_LENGTH 10
#define CONFIG_NUM_SWITCHES 22

#define CONFIG_MAX_UPCOMING_SENSORS 10

int switch_index(int sw);
int switch_index_inverse(int sw);

#endif

