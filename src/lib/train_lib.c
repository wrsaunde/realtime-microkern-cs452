#include <userspace.h>

#include <lib/all.h>
#include <config/key_words.h>
#include <config/train_constants.h>


#include <tasks/train_data.h>
#include <tasks/track_node.h>



int switch_index(int sw) {
	if(sw >= 153 && sw <= 156) {
		return sw - 153 + 18;
	} else if (sw >= 1 && sw <= 18) {
		return sw - 1;
	} else {
		return -1;
	}
}

int switch_index_inverse(int sw) {
	if(sw >= 18 && sw <= 21) {
		return sw - 18 + 153;
	} else if(sw >= 0 && sw <= 17) {
		return sw + 1;
	} else {
		return -1;
	}
}

char sensor_letter(int sensor) {
	return (char) ('A' + ((sensor) / 16));
}

int sensor_number(int sensor) {
	return (((sensor) % 16) + 1);
}

int sensor_str2num(const char* input, int* output){
	char num_portion[3];
	int num_value;
	char c_portion;
	
	//check if length is too short
	if( input[0] == '\0' || input[1] == '\0' ){ return FALSE; }
	
	//check the character portion
	c_portion = input[0];
	if( c_portion < 'A' || c_portion > 'E' ){
		if( c_portion < 'a' || c_portion > 'e' ){
			return FALSE;
		}
		c_portion = 'A' + ( c_portion - 'a' );
	}
	
	//check the number portion
	if( input[2] == '\0' ){
		num_portion[0] = input[1];
		num_portion[1] = '\0';
	}else if( input[3] == '\0' ){
		num_portion[0] = input[1];
		num_portion[1] = input[2];
		num_portion[2] = '\0';
	}else{
		return FALSE;
	}
	if(!str_is_integer(num_portion)){ return FALSE; }
	num_value = str_atoi(num_portion);
	if(num_value < 1 || num_value > 16){ return FALSE; }
	
	//both character and number are valid, calculate the proper num
	*output = (16 * (int)(c_portion - 'A')) + (num_value - 1);
	return TRUE;	
}

int sensor_num2str(int input, char* output){
	output[0] = sensor_letter(input);
	
	int num = sensor_number(input);
	
	if( num / 10 ){
		output[1] = '1';
		output[2] = (char) '0' + (num % 10);
		output[3] = '\0';
	}else{
		output[1] = (char) num;
		output[2] = '\0';
	}
	
	return TRUE;
}

int track_str2num(struct track_node * track, char * input, int * output) {
	int start = 0, len = 0, i = 0;
	if((input[0] == 'B' || input[0] == 'b') && (input[1] == 'R' || input[1] == 'r')) {
		start = 80;
		len = CONFIG_NUM_SWITCHES;
	} else if((input[0] == 'M' || input[0] == 'm') && (input[1] == 'R' || input[1] == 'r')) {
		start = 81;
		len = CONFIG_NUM_SWITCHES;
	} else if((input[0] == 'E' || input[0] == 'e') && (input[1] == 'N' || input[1] == 'n') ) {
		start = 124;
		len = 10;
	} else if((input[0] == 'E' || input[0] == 'e')  && (input[1] == 'X' || input[1] == 'x')) {
		start = 125;
		len = 10;
	} else {
		return sensor_str2num(input, output);
	}
	for(i = 0; i < len; i++) {
		if(strcmp(track[start + i*2].name + 2, input + 2) == 0) {
			*output = start + i*2;
			return TRUE;
		}
	}
	return FALSE;
}

int track_num2str(struct track_node * track, int input, char * output) {
	output[0] = '\0';
	if(input < 144 && (input != 0 && track[input].index == 0)) {
		safestrcpy(output, (char *) track[input].name, 6);
		return TRUE;
	}
	return FALSE;
}

char * strtrack_num2str(struct track_node * track, int input, char * output) {
	if(input < 144 && (input != 0 && track[input].index == 0) ) {
		safestrcpy(output, (char *) track[input].name, 6);
		return output;
	}
	output[0] = '\0';
	return output;
}

int speed_mdi_to_logical(int speed, int oldspeed) {
	if(oldspeed > speed * 2 + 1) {
		//Decellerating into new speed
		oldspeed = speed * 2 + 1;
	} else if(oldspeed < speed * 2) {
		//Accellerating into new speed
		oldspeed = speed * 2;
	}
	return oldspeed;
}

int speed_logical_to_mdi(int speed) {
	return speed / 2;
}





//returns distance traveled while accelerating, subtracts acceleration time
//from the time variable until it is zero
int distance_while_accelerating(struct train_calibration * c, int start_speed, int end_speed, int * time){
	
	int total_expected_time = 0;
	int vinit = 0;
	int vfinal = 0;
	int speeddiff = start_speed - end_speed;
	if(speeddiff < 0) { speeddiff = -speeddiff; }
	if(speeddiff < 5) {
		//Correct for trains get stuck
		// 2 * distance / Vinit
		total_expected_time = 2 * c->stopdist[5] * c->timetotal[5] / c->disttotal[5];
		
		if(c->disttotal[5] == 0){
			vinit = 0;
		}else{
			vinit = c->timetotal[5] / c->disttotal[5];
		}
		if(c->disttotal[5 + speeddiff] == 0){
			vfinal = 0;
		}else{
			vfinal = c->timetotal[5 + speeddiff] / c->disttotal[5 + speeddiff];
		}
	} else {
		//We have good data
		// 2 * distance / Vinit
		total_expected_time = 2 * c->stopdist[speeddiff] * c->timetotal[speeddiff] / c->disttotal[speeddiff];
		if(c->disttotal[start_speed] == 0){
			vinit = 0;
		}else{
			vinit = c->timetotal[start_speed] / c->disttotal[start_speed];
		}
		if(c->disttotal[end_speed] == 0){
			vfinal = 0;
		}else{
			vfinal = c->timetotal[end_speed] / c->disttotal[end_speed];
		}
	}
	
	//update the time variable and actually calculate our length of accel
	int time_accelerating = *time;
	if(total_expected_time >= *time){
		*time = 0;
	}else{
		time_accelerating = total_expected_time;
		*time = *time - total_expected_time;
	}
	
	int dist_traveled = 0;
	
	//calculate the distance traveled
	//Telapsed * (Vinit + [(Vfinal - Vinit) * Telapsed] / (2 * Texpect))
	dist_traveled = time_accelerating  * (vinit + (vfinal - vinit) * time_accelerating / (2 * total_expected_time));
	return dist_traveled;	
}

//TODO: FIX ME!!!!
int train_accel(struct train_data* DATA, int vinit, int vfinal){
	if(vfinal - vinit < 0){
		return 0;
	}else{
		return 0;
	}
	return 0;
}



int offset_front( struct train_data* DATA ){
	switch(DATA->state.direction){
		case DIRECTION_FORWARD:
			return DATA->calibration.front_len;
			break;
		case DIRECTION_BACKWARD:
			return DATA->calibration.back_len + DATA->calibration.pickup_len;
			break;
		default:
			//assume our train is as long as possible, return that case
			return MAX(DATA->calibration.back_len, DATA->calibration.front_len) + DATA->calibration.pickup_len;
			break;
	}
}


int offset_rear( struct train_data* DATA ){
	switch(DATA->state.direction){
		case DIRECTION_FORWARD:
			return DATA->calibration.back_len;
			break;
		case DIRECTION_BACKWARD:
			return DATA->calibration.front_len + DATA->calibration.pickup_len;
			break;
		default:
			//assume our train is as long as possible, return that case
			return MAX(DATA->calibration.back_len, DATA->calibration.front_len) + DATA->calibration.pickup_len;
			break;
	}
}

int train_alias( int tnum ){
	if(tnum >= 100){
		return tnum / 10;
	}
	return tnum;
}

