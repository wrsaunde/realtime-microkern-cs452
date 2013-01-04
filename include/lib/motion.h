#ifndef __LIB_MOTION_H__
#define __LIB_MOTION_H__

struct velocity {
	int d;
	int t;
};
//library functions for motion calculations

int mot_distance( int jerk_const, struct velocity vi, struct velocity vf, int t );
struct velocity mot_veloc( int jerk_const, struct velocity vi, struct velocity vf, int t );
int mot_time( int jerk_const, struct velocity vi, struct velocity vf );

int mot_accel_dist_to_time( int jerk_const, struct velocity vi, struct velocity vf, int dist );

//calculate the distance travelled during an acceleration period
//if the time provided is greater than the time to accelerate
//only the distance where acceleration should have finished will
//be returned
int mot_accel_dist( int jerk_const, struct velocity vi, struct velocity vf, int t );

//calculate the time to accelerate
int mot_accel_time( int jerk_const, struct velocity vi, struct velocity vf );

//calculate the velocity at a time during acceleration
struct velocity mot_accel_veloc( int jerk_const, struct velocity vi, struct velocity vf, int t );

#endif
