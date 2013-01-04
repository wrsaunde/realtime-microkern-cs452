#include <userspace.h>

void task_test_zombie_reclaim() {

	struct two_number_message init_msg;
	int block_size;
	int num_blocks;
	
	//initialize the test
	int tid;
	Receive(&tid, (char *) &init_msg, sizeof(init_msg));
	Reply(tid, (char *) 0, 0);
	block_size = init_msg.num1;
	num_blocks = init_msg.num2;
	
	int tids[0xFF];
	
	int i = 0;
	int j = 0;
	for(i = 0; i < num_blocks; i++){
		for(j = 0; j < block_size; j++){
			tids[j] = Create(PRIORITY_TEST_ZOMB_SUB, &task_test_zombie_reclaim_sub);
			if( tids[j] < 1 ){
				CommandOutput("[ZOMB] FAIL: CREATE[i:%d][j:%d][tid:%d]", i, j, tids[j]);
				Quit();
			}
		}
		
		CommandOutput("[ZOMB] BLOCK CREATED[%d] - FIRST[%x] LAST[%x]",i, tids[0], tids[j - 1]);
		
		for(j = 0; j < block_size; j++){
			init_msg.num1 = j;
			int status = Send(tids[j], (char *) &init_msg, sizeof(init_msg), (char *) &init_msg, sizeof(init_msg));
			if( status < 0 || init_msg.num1 != tids[j] || init_msg.num2 != j * 2){
				CommandOutput("[ZOMB] FAIL: SEND");
				Quit();
			}
		}
		
		CommandOutput("[ZOMB] ALL RETURNED[%d]", i);
	}
	
	
	CommandOutput("[ZOMB] COMPLETE");
	
	Exit();
}

void task_test_zombie_reclaim_sub(){
	struct two_number_message init_msg;
	
	int recvtid = 0;
	Receive(&recvtid, (char *) &init_msg, sizeof(init_msg));
	init_msg.num2 = init_msg.num1 * 2;
	init_msg.num1 = MyTid();
	Reply(recvtid, (char *) &init_msg, sizeof(init_msg));
	
	Exit();
}

