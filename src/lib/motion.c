#include <lib/all.h>
#include <config/user.h>

int mot_time( int jerk_const, struct velocity vi, struct velocity vf ) {
	if( vi.t == 0 ) {
		vi.t = 1;
		vi.d = 0;
	}
	if( vf.t == 0 ) {
		vf.t = 1;
		vf.d = 0;
	}

	int accel_time = mot_accel_time( jerk_const, vi, vf );
	if( accel_time > 0 ) {
		return CONFIG_TRAIN_COMMAND_DELAY + accel_time;
	}
	return 0;
}


int mot_distance( int jerk_const, struct velocity vi, struct velocity vf, int t ) {
	if( vi.t == 0 ) {
		vi.t = 1;
		vi.d = 0;
	}
	if( vf.t == 0 ) {
		vf.t = 1;
		vf.d = 0;
	}

	int accel_length = mot_accel_time( jerk_const, vi, vf );

	int vi_time = 0;
	int accel_time = 0;
	int vf_time = 0;

	if(t <= CONFIG_TRAIN_COMMAND_DELAY){
		vi_time = t;
	}else if(t <= CONFIG_TRAIN_COMMAND_DELAY + accel_length){
		vi_time = CONFIG_TRAIN_COMMAND_DELAY;
		accel_time = t - CONFIG_TRAIN_COMMAND_DELAY;
	}else{
		vi_time = CONFIG_TRAIN_COMMAND_DELAY;
		accel_time = accel_length;
		vf_time = t - CONFIG_TRAIN_COMMAND_DELAY - accel_length;
	}

	int accel_offset = mot_accel_dist( jerk_const, vi, vf, accel_time );
	int const_offset = vi_time * vi.d / vi.t + vf_time * vf.d / vf.t;

	return accel_offset + const_offset;
}


struct velocity mot_veloc( int jerk_const, struct velocity vi, struct velocity vf, int t ) {
	if( vi.t == 0 ) {
		vi.t = 1;
		vi.d = 0;
	}
	if( vf.t == 0 ) {
		vf.t = 1;
		vf.d = 0;
	}

	int accel_length = mot_accel_time( jerk_const, vi, vf );

	if(t <= CONFIG_TRAIN_COMMAND_DELAY){
		return vi;
	}else if(t <= CONFIG_TRAIN_COMMAND_DELAY + accel_length){
		return mot_accel_veloc( jerk_const, vi, vf, t - CONFIG_TRAIN_COMMAND_DELAY);
	}else{
		return vf;
	}
}


/*
 Functions Following Are Internal to the motion library
 */

int mot_accel_time( int jerk_const, struct velocity vi, struct velocity vf ) {
	if( vi.t == 0 ) {
		vi.t = 1;
		vi.d = 0;
	}
	if( vf.t == 0 ) {
		vf.t = 1;
		vf.d = 0;
	}
	assert( (vf.t * vi.t) >= 0, "TIME WAS NEGATIVE" );
	assert( 6 * jerk_const > 0, "JERK WAS <= 0" );
	return SQRT( 6 * jerk_const ) * SQRT( ABS( vf.d * vi.t - vi.d * vf.t ) ) / SQRT( vf.t * vi.t );
}


int mot_accel_dist( int jerk_const, struct velocity vi, struct velocity vf, int t ) {
	int tf = mot_accel_time( jerk_const, vi, vf );
	if( tf == 0 ) {
		return 0;
	}
	if( vi.t == 0 ) {
		vi.t = 1;
		vi.d = 0;
	}
	if( vf.t == 0 ) {
		vf.t = 1;
		vf.d = 0;
	}

	int vnumer = (vf.d * vi.t - vi.d * vf.t);
	int vdenom = (vf.t * vi.t);

	//FOR CAST SEMANTICS, RTFM
	double term1 = (double)vnumer * t * t * t * t / vdenom / tf / tf / tf / (-2);
	double term2 = (double)vnumer * t * t * t / vdenom / tf / tf;
	double term3 = (double)vi.d * t / vi.t;

	return ABS( (int)(term1 + term2 + term3) );
}


int mot_accel_dist_to_time( int jerk_const, struct velocity vi, struct velocity vf, int dist ) {
	if( vi.t == 0 ) {
		vi.t = 1;
		vi.d = 0;
	}
	if( vf.t == 0 ) {
		vf.t = 1;
		vf.d = 0;
	}
	int tf = mot_time( jerk_const, vi, vf );

	int t = tf / 2;
	int tlast = 0;

	while( ABS( t - tlast ) >= 1 ) {
		tlast = t;
		int x = dist - mot_distance( jerk_const, vi, vf, t );
		struct velocity vt = mot_veloc( jerk_const, vi, vf, t );

		if( vt.d == 0 ) {
			t = t;
		} else {
			t = t + x * vt.t / vt.d;
		}
		if( t < 0 ) {
			t = 0;
		}
	}

	return MAX( t, tlast );
}


struct velocity mot_accel_veloc( int jerk_const, struct velocity vi, struct velocity vf, int t ) {
	int tf = mot_accel_time( jerk_const, vi, vf );
	if( tf == 0 ) {
		return vi;
	}
	if( vi.t == 0 ) {
		vi.t = 1;
		vi.d = 0;
	}
	if( vf.t == 0 ) {
		vf.t = 1;
		vf.d = 0;
	}

	int vnumer = (vf.d * vi.t - vi.d * vf.t);
	int vdenom = (vf.t * vi.t);

	//FOR CAST SEMANTICS, RTFM
	double term1 = (double)(-2) * vnumer * t * t * t / vdenom / tf / tf / tf;
	double term2 = (double)3 * vnumer * t * t / vdenom / tf / tf;
	double term3 = (double)vi.d / vi.t;

	struct velocity return_val;

	int multiplier = 100000;
	return_val.d = ABS( (int)((term1 + term2 + term3) * multiplier) );
	return_val.t = multiplier;
	return return_val;
}
