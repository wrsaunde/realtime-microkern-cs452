#include <userspace.h>

void task_test_clock_accuracy() {
	//int i = 0;
	int tid = 0;
	
	tid = Create(5,&task_system_nameserver);
	//assert(tid == NAMESERVER_TID,"tid == NAMESERVER_TID");
	
	tid = Create(5,&task_system_clock_server);
	tid = Create(9,&task_system_clock_notifier);
	
	int print_interval = 100;
	
	int time = -1;
	int time2 = -1;
	int time3 = -1;
	
	while(1) {
		time2 = Time();
		
		assert(time2 <= time3 + 1,"time2 <= time3 + 1");
		time3 = time2;
		
		if(time2 == time + print_interval){
			time = time2;
			bwprintf(COM2, "%d\n", time);
		}
	}
	Exit();
}

