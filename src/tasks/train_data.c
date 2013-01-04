#include <tasks/train_data.h>

static void *memset(void *s, int c, unsigned int n) {
  unsigned char *p = s;
  while(n --> 0) { *p++ = (unsigned char) c; }
  return s;
}

void init_train_calibration(struct train_calibration * d, int number) {
	memset(d, 0, sizeof(struct train_calibration));
	d->number = number;
	switch(number) {
		case 21:
			d->length = 191;
			d->pickup_len = 51;
			d->front_len = 22;
			d->back_len = 118;

			d->speed_high = 14;
			d->speed_med = 10;
			d->speed_low = 10;

			/*
			//acceleration constants
			dist = (1/2) * (vfinal + vinit) * t
			t = 2 * dist / (vfinal - vinit)
			t = 426; //speed 14
			t = 324 and 337 //speed 10
			t = 289 and 305	//speed 9
			t = 225 and	240	//speed 7

			a =  vi / t
			a = 0.00848967//speed 14
			a = 0.00639541 and 0.00640124//speed 10
			a = 0.00659142 and 0.00589615//speed 9
			a = 0.00562470 and 0.00560917//speed 7
			*/

			//d->accel_down_const = 6;
			//d->accel_up_const = 6;

			d->jerk_const_up = 9001;
			d->jerk_const_down = 9001;

			//Train 21
			d->disttotal[0] = 0; 	d->timetotal[0] = 17325;	//0
			d->disttotal[1] = 0; 	d->timetotal[1] = 0;	//0
			d->disttotal[2] = 0; 	d->timetotal[2] = 0;	//1
			d->disttotal[3] = 0; 	d->timetotal[3] = 0;	//1
			d->disttotal[4] = 0; 	d->timetotal[4] = 0;	//2
			d->disttotal[5] = 2246; 	d->timetotal[5] = 11151;	//2
			d->disttotal[6] = 3242; 	d->timetotal[6] = 10479;	//3
			d->disttotal[7] = 11582; 	d->timetotal[7] = 32254;	//3
			d->disttotal[8] = 5433; 	d->timetotal[8] = 11103;	//4
			d->disttotal[9] = 13550; 	d->timetotal[9] = 22624;	//4
			d->disttotal[10] = 7773; 	d->timetotal[10] = 11104;	//5
			d->disttotal[11] = 17619; 	d->timetotal[11] = 22041;	//5
			d->disttotal[12] = 10893; 	d->timetotal[12] = 11118;	//6
			d->disttotal[13] = 24705; 	d->timetotal[13] = 22501;	//6
			d->disttotal[14] = 14152; 	d->timetotal[14] = 11056;
			d->disttotal[15] = 9844; 	d->timetotal[15] = 7625;
			d->disttotal[16] = 21938; 	d->timetotal[16] = 14817;	//8
			d->disttotal[17] = 23370; 	d->timetotal[17] = 13759;	//8
			d->disttotal[18] = 18075; 	d->timetotal[18] = 10051; //9
			d->disttotal[19] = 27048; 	d->timetotal[19] = 14199; //9
			d->disttotal[20] = 21500; 	d->timetotal[20] = 10000;
			//d->disttotal[20] = 17700; 	d->timetotal[20] = 8524; //10
			d->disttotal[21] = 31435; 	d->timetotal[21] = 14572;//10
			d->disttotal[22] = 16710; 	d->timetotal[22] = 7492;
			d->disttotal[23] = 16315; 	d->timetotal[23] = 7309;
			d->disttotal[24] = 27400; 	d->timetotal[24] = 10000;	//12
			//d->disttotal[24] = 17700; 	d->timetotal[24] = 6551;	//12
			d->disttotal[25] = 17700; 	d->timetotal[25] = 6196;	//12
			d->disttotal[26] = 31347; 	d->timetotal[26] = 10286;	//13
			d->disttotal[27] = 21712; 	d->timetotal[27] = 6693;	//13
			d->disttotal[28] = 17896; 	d->timetotal[28] = 5057;
			d->disttotal[29] = 0; 	d->timetotal[29] = 0;
			//End of Train 21
			break;

		case 23:
			d->length = 215;
			d->pickup_len = 51;
			d->front_len = 26;
			d->back_len = 138;

			d->speed_high = 14;
			d->speed_med = 7;
			d->speed_low = 7;

			d->jerk_const_up = 2166;//3050; //650/1000000 // 6/6250
			d->jerk_const_down = 1041;//4000; //250/1000000 // 6/13000

			//Train 23
			d->disttotal[0] = 0; 	d->timetotal[0] = 0;
			d->disttotal[1] = 0; 	d->timetotal[1] = 0;
			d->disttotal[2] = 0; 	d->timetotal[2] = 0;
			d->disttotal[3] = 0; 	d->timetotal[3] = 0;
			d->disttotal[4] = 0; 	d->timetotal[4] = 0;
			d->disttotal[5] = 0; 	d->timetotal[5] = 0;
			d->disttotal[6] = 0; 	d->timetotal[6] = 0;
			d->disttotal[7] = 0; 	d->timetotal[7] = 0;
			d->disttotal[8] = 0; 	d->timetotal[8] = 0;
			d->disttotal[9] = 0; 	d->timetotal[9] = 0;
			d->disttotal[10] = 0; 	d->timetotal[10] = 0;
			d->disttotal[11] = 0; 	d->timetotal[11] = 0;
			d->disttotal[12] = 0; 	d->timetotal[12] = 0;
			d->disttotal[13] = 0; 	d->timetotal[13] = 0;
			d->disttotal[14] = 37700; 	d->timetotal[14] = 10000;
			d->disttotal[15] = 0; 	d->timetotal[15] = 0;
			d->disttotal[16] = 16074; 	d->timetotal[16] = 5053;	//8-
			d->disttotal[17] = 19465; 	d->timetotal[17] = 5579;	//8
			d->disttotal[18] = 12878; 	d->timetotal[18] = 3500;	//9-
			d->disttotal[19] = 21415; 	d->timetotal[19] = 5459;	//9
			d->disttotal[20] = 40922; 	d->timetotal[20] = 10150;	//10-
			d->disttotal[21] = 23370; 	d->timetotal[21] = 5438;	//10
			d->disttotal[22] = 24426; 	d->timetotal[22] = 5418;	//11-
			d->disttotal[23] = 25320; 	d->timetotal[23] = 5492;	//11
			d->disttotal[24] = 32718; 	d->timetotal[24] = 7252;	//12-
			d->disttotal[25] = 27285; 	d->timetotal[25] = 5505;	//12
			d->disttotal[26] = 56088; 	d->timetotal[26] = 10934;	//13-
			d->disttotal[27] = 60003; 	d->timetotal[27] = 11412;	//13
			d->disttotal[28] = 62962; 	d->timetotal[28] = 11583;	//14
			d->disttotal[29] = 0; 	d->timetotal[29] = 0;
			
			//End of Train 23
			break;			

		case 24:
			d->length = 219;
			d->pickup_len = 51;
			d->front_len = 25;
			d->back_len = 143;

			d->speed_high = 14;
			d->speed_med = 10;
			d->speed_low = 10;

			d->jerk_const_up = 9900;
			d->jerk_const_down = 2600;

			d->disttotal[0] = 0; 	d->timetotal[0] = 0;
			d->disttotal[1] = 0; 	d->timetotal[1] = 0;
			d->disttotal[2] = 0; 	d->timetotal[2] = 0;
			d->disttotal[3] = 0; 	d->timetotal[3] = 0;
			d->disttotal[4] = 0; 	d->timetotal[4] = 0;
			d->disttotal[5] = 0; 	d->timetotal[5] = 0;
			d->disttotal[6] = 0; 	d->timetotal[6] = 0;
			d->disttotal[7] = 0; 	d->timetotal[7] = 0;
			d->disttotal[8] = 0; 	d->timetotal[8] = 0;
			d->disttotal[9] = 0; 	d->timetotal[9] = 0;
			d->disttotal[10] = 12324; 	d->timetotal[10] = 5357;
			d->disttotal[11] = 45307; 	d->timetotal[11] = 19699;
			d->disttotal[12] = 15208; 	d->timetotal[12] = 5420;
			d->disttotal[13] = 15597; 	d->timetotal[13] = 5539;
			d->disttotal[14] = 18696; 	d->timetotal[14] = 5494;
			d->disttotal[15] = 18696; 	d->timetotal[15] = 5493;
			d->disttotal[16] = 21808; 	d->timetotal[16] = 5553;
			d->disttotal[17] = 21712; 	d->timetotal[17] = 5522;
			d->disttotal[18] = 24741; 	d->timetotal[18] = 5543;
			d->disttotal[19] = 24426; 	d->timetotal[19] = 5452;
			d->disttotal[20] = 26690; 	d->timetotal[20] = 5507;
			d->disttotal[21] = 28044; 	d->timetotal[21] = 5569;
			d->disttotal[22] = 30365; 	d->timetotal[22] = 5442;
			d->disttotal[23] = 30846; 	d->timetotal[23] = 5524;
			d->disttotal[24] = 32718; 	d->timetotal[24] = 5415;
			d->disttotal[25] = 33390; 	d->timetotal[25] = 5497;
			d->disttotal[26] = 32431; 	d->timetotal[26] = 5459;
			d->disttotal[27] = 33487; 	d->timetotal[27] = 5523;
			d->disttotal[28] = 38103; 	d->timetotal[28] = 6760;
			d->disttotal[29] = 0; 	d->timetotal[29] = 0;

			break;
		case 210:	//TRAIN 21b
			d->length = 191;
			d->pickup_len = 51;
			d->front_len = 22;
			d->back_len = 118;

			d->speed_high = 14;
			d->speed_med = 7;
			d->speed_low = 5;

			d->jerk_const_up = 8000;
			d->jerk_const_down = 4364;

			d->disttotal[0] = 0; 	d->timetotal[0] = 0;
			d->disttotal[1] = 0; 	d->timetotal[1] = 0;
			d->disttotal[2] = 0; 	d->timetotal[2] = 0;
			d->disttotal[3] = 0; 	d->timetotal[3] = 0;
			d->disttotal[4] = 0; 	d->timetotal[4] = 0;
			d->disttotal[5] = 0; 	d->timetotal[5] = 0;
			d->disttotal[6] = 0; 	d->timetotal[6] = 0;
			d->disttotal[7] = 0; 	d->timetotal[7] = 0;
			d->disttotal[8] = 0; 	d->timetotal[8] = 0;
			d->disttotal[9] = 0; 	d->timetotal[9] = 0;
			d->disttotal[10] = 6921; 	d->timetotal[10] = 5578;
			d->disttotal[11] = 12590; 	d->timetotal[11] = 8719;
			d->disttotal[12] = 8973; 	d->timetotal[12] = 5423;
			d->disttotal[13] = 10010; 	d->timetotal[13] = 5436;
			d->disttotal[14] = 6747; 	d->timetotal[14] = 3350;
			d->disttotal[15] = 10518; 	d->timetotal[15] = 5009;
			d->disttotal[16] = 8292; 	d->timetotal[16] = 3488;
			d->disttotal[17] = 9348; 	d->timetotal[17] = 3540;
			d->disttotal[18] = 10719; 	d->timetotal[18] = 3573;
			d->disttotal[19] = 11595; 	d->timetotal[19] = 3581;
			d->disttotal[20] = 12852; 	d->timetotal[20] = 3510;
			d->disttotal[21] = 14462; 	d->timetotal[21] = 3470;
			d->disttotal[22] = 16824; 	d->timetotal[22] = 3552;
			d->disttotal[23] = 18291; 	d->timetotal[23] = 3550;
			d->disttotal[24] = 19398; 	d->timetotal[24] = 3513;
			d->disttotal[25] = 21795; 	d->timetotal[25] = 3580;
			d->disttotal[26] = 23370; 	d->timetotal[26] = 3543;
			d->disttotal[27] = 24514; 	d->timetotal[27] = 3561;
			d->disttotal[28] = 33491; 	d->timetotal[28] = 4852;
			d->disttotal[29] = 0; 	d->timetotal[29] = 0;
			break;
		default:
			break;
	}
}
