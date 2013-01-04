#include <userspace.h>

int WorkerDelay(int delay) {
	struct two_number_message init_msg;
	int status, tid;
	
	//create, initialize, and queue the courier
	tid = Create(PRIORITY_DELAY_WORKER, &task_system_worker_delay);
	
	if(tid < 0) {
		return -1;
	}
	
	init_msg.num1 = 0;		//type
	init_msg.num2 = delay;	//length
	
	//send the initialization message
	status = Send(tid, (char *) &init_msg, sizeof(init_msg), (char *) 0, 0);
	
	if(status < 0) {
		return -2;
	}
	
	return 0;
}

int WorkerPeriodicDelay(int delay) {
	struct two_number_message init_msg;
	int status, tid;
	
	//create, initialize, and queue the courier
	tid = Create(PRIORITY_DELAY_WORKER, &task_system_worker_periodic_delay);
	
	if(tid < 0) {
		return -1;
	}
	
	init_msg.num1 = 0;		//type
	init_msg.num2 = delay;	//length
	
	//send the initialization message
	status = Send(tid, (char *) &init_msg, sizeof(init_msg), (char *) 0, 0);
	
	if(status < 0) {
		return -2;
	}
	
	return 0;
}

int WorkerDelayUntil(int delay) {
	struct two_number_message init_msg;
	int status, tid;
	
	//create, initialize, and queue the courier
	tid = Create(PRIORITY_DELAY_WORKER, &task_system_worker_delay);
	
	if(tid < 0) {
		return -1;
	}
	
	init_msg.num1 = 1;		//type
	init_msg.num2 = delay;	//length
	
	//send the initialization message
	status = Send(tid, (char *) &init_msg, sizeof(init_msg), (char *) 0, 0);
	
	if(status < 0) {
		return -2;
	}
	
	return 0;
}

int WorkerDelayWithId(int delay, int id) {
	struct two_number_message init_msg;
	int status, tid;
	
	//create, initialize, and queue the courier
	tid = Create(PRIORITY_DELAY_WORKER, &task_system_worker_delay_with_id);
	
	if(tid < 0) {
		return -1;
	}
	
	init_msg.num1 = id;		//type
	init_msg.num2 = delay;	//length
	
	//send the initialization message
	status = Send(tid, (char *) &init_msg, sizeof(init_msg), (char *) 0, 0);
	
	if(status < 0) {
		return -2;
	}
	
	return 0;
}

int WorkerDelayUntilWithId(int delay, int id) {
	struct two_number_message init_msg;
	int status, tid;
	
	//create, initialize, and queue the courier
	tid = Create(PRIORITY_DELAY_WORKER, &task_system_worker_delay_until_with_id);
	
	if(tid < 0) {
		return -1;
	}
	
	init_msg.num1 = id;		//type
	init_msg.num2 = delay;	//length
	
	//send the initialization message
	status = Send(tid, (char *) &init_msg, sizeof(init_msg), (char *) 0, 0);
	
	if(status < 0) {
		return -2;
	}
	
	return 0;
}
