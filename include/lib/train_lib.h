#ifndef __TRAINLIB_H__
#define __TRAINLIB_H__

#include <tasks/train_data.h>
#include <tasks/track_node.h>
#include <tasks/track_data.h>

struct train_calibration;

int switch_index(int sw);

int switch_index_inverse(int sw);

char sensor_letter(int sensor);

int sensor_number(int sensor);

int sensor_str2num(const char* input, int* output);

int sensor_num2str(int input, char* output);

int track_str2num(struct track_node * track, char * input, int * output);

int track_num2str(struct track_node * track, int input, char * output);

char * strtrack_num2str(struct track_node * track, int input, char * output);

int distance_while_accelerating(struct train_calibration * c, int start_speed, int end_speed, int * time);

int speed_mdi_to_logical(int speed, int oldspeed);

int speed_logical_to_mdi(int speed);

int train_accel(struct train_data* DATA, int vinit, int vfinal);

//return the offset from the train location to the front edge of the train
int offset_front( struct train_data* DATA );

//return the offset from the train location to the rear edge of the train
int offset_rear( struct train_data* DATA );

int train_alias( int tnum );


#endif
