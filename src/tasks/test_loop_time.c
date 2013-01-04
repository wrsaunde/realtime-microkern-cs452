#include <userspace.h>

void task_test_loop_time() {
	int t1 = 0;
	int t2 = 0;
	unsigned int iterations = 1000000;
	
	int h = 0;
	
	t1 = Time();
	
	unsigned int i = 0;
	for(i = 0; i < iterations; i++){
		h = h * 2;
	}
	
	t2 = Time();
	
	int diff = t2 - t1;
	
	CommandOutput("[LOOPTEST] Took [%d] ticks", diff);


	//find how many time calls can fit in a single tick
	int times[10];
	int max = 0;
	int min = 0;
	int j  = 0;
	for(j = 0; j < 10; j++){
		times[j] = 0;
	}
	j = 0;
	while(j < 10){
		int tinit = Time();
		while(tinit == Time()){
			;
		}
		int tfin = Time() + 20;

		while(tfin > Time()){
			times[j]++;
		}
		times[j] = times[j] / 20;
		j++;
	}

	max = times[0];
	min = times[0];
	for(j = 0; j < 10; j++){
		max = MAX(max, times[j]);
		min = MIN(min, times[j]);
	}
	CommandOutput("[LOOPTEST] # of Time() calls per tick -> MAX[%d] MIN[%d]", max, min);

	
	Exit();
}

