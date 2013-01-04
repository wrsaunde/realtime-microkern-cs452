#include <kernspace.h>

void initialize_interrupt_vec() {
	//Install swi handler
	int * y = (int *) 0x28;
	*y = (int) baseaddr() + (int) &kerent;
	
	//Install interrupt handler
	y = (int *) 0x38;
	*y = (int) baseaddr() + (int) &kerentirq;
}

void nops_ftw() {
	//Delay loop
	int i;
	for(i = 0; i < 55;) {
		i++;
	}	
}

void initialize_hardware() {
	//Invalidate Instruction Cache
	asm("MOV r10, #0");
	asm("MCR p15, 0, r10, c7, c5, 0");
	//Enable the Instruction Cache
	asm("MRC p15, 0, r10, c1, c0, 0");	//get the current value of the register
	asm("MOV r9, #1");
	asm("MOV r9, r9, LSL#30");	//generate the proper bits in r9 for fast clock
	asm("ORR r10, r10, #4096");	//or the bits for the instruction cache
	asm("ORR r10, r10, r9");	//or the bits for the fast clock
	//asm("ORR r10, r10, #2");	//or the bits for the data cache
	asm("MCR p15, 0, r10, c1, c0, 0");


	int buf;
	int * line;
	int *high, *low, *ctlr, *mdmctl;	
	unsigned int* intenable;
	unsigned int* t3;
		
	//set up the timer
	t3 = (unsigned int*) TIMER3_BASE;	//get the timer address
	//disable the timer
	t3[2] = 0;
	//load the new value
	t3[0] = TICK_INTERVAL - 1;
	//enable the timer in periodic mode at 2kHz ticks
	t3[2] = ENABLE_MASK | MODE_MASK;


	
	//Enable Interrupts in VIC
	intenable = (unsigned int*) (VIC2_BASE + VIC_INTENABLE);
	*intenable = VIC2_TIMER_MASK | VIC2_UART2_MASK | VIC2_UART1_MASK;

	//Set interrupts on for UART2
	ctlr = (int *)( UART2_BASE + UART_CTLR_OFFSET );
	buf = *ctlr;
	*ctlr = buf | RIEN_MASK | RTIEN_MASK;// | TIEN_MASK;
	
	nops_ftw();
	
	//Set interrupts on for UART1
	ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
	buf = *ctlr;
	*ctlr = buf | RIEN_MASK;// | RTIEN_MASK;
	
	nops_ftw();
	
	//Set speed for UART2
	high = (int *)( UART2_BASE + UART_LCRM_OFFSET );
	low = (int *)( UART2_BASE + UART_LCRL_OFFSET );	
	*high = 0x0;
	nops_ftw();
	*low = 0x3;

	//Disable fifo for UART2
	line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
	buf = *line;
	*line = buf | FEN_MASK;	
	
	nops_ftw();
	
	nops_ftw();
	
	//Set speed for UART1
	high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
	low = (int *)( UART1_BASE + UART_LCRL_OFFSET );	
	*high = 0;
	nops_ftw();
	*low = 191;
	
	nops_ftw();
	
	mdmctl = (int *)( UART1_BASE + UART_MDMCTL_OFFSET);	
	*mdmctl = (*mdmctl | 1);

	nops_ftw();	
	
	//Disable fifo for UART1
	line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
	buf = *line;
	buf = buf | STP2_MASK;
	*line = buf & ~FEN_MASK;
	
	initialize_interrupt_vec();
}

void cleanup_hardware() {
	//set up the timer
	unsigned int* t3 = (unsigned int*) TIMER3_BASE;	//get the timer address
	//disable the timer
	t3[2] = 0;	
	
	unsigned int* intdisable = (unsigned int*) VIC2_BASE + VIC_INTENABLE;
	*intdisable = VIC2_TIMER_MASK | VIC2_UART2_MASK; //| 0x00100000;;	
}

void initialize_globals(struct kern_globals* GLOBAL){
	//initialize the TD space
	GLOBAL->next_free_td = 0;
	GLOBAL->free_list = NULL;
	
	//initialize the load offset
	GLOBAL->load_offset = baseaddr();
		
	//initialize the task id counter
	GLOBAL->next_task_id = 1;
	
	initialize_task_descriptors(GLOBAL);
	initialize_scheduler(GLOBAL);
	initialize_events(GLOBAL);
}

void initialize_task_descriptors(struct kern_globals* GLOBAL){
	/*Position user stacks in memory*/
	
	/*leave space for kernel stack*/
	unsigned int stackpos = MEMORY_SIZE - KERNEL_PADDING - 4;
	
	//Leave room for shared memory too
	stackpos -= SHM_TOTAL_SIZE;
	
	//for each task descriptor
	int i = 0;
	for(i = 0; i <= TID_INDEX_MASK; i++) {
		//initialize the memory
		if(stackpos < GLOBAL->load_offset + KERNEL_PADDING) {
			bwprintf(COM2, "We're out of memory for user tasks!!! We only have room for the first %d!!\r\n", i);
			bwgetc(COM2);
			i = TID_INDEX_MASK;
		} else {
			GLOBAL->USER_TDS[i].stack_space_top = (char*)stackpos;
			stackpos -= STACK_SIZE;	
			GLOBAL->USER_TDS[i].stack_space_bottom = (char*)stackpos;		
		}
		GLOBAL->USER_TDS[i].send_Q_size = 0;
		
	}

}

void initialize_events(struct kern_globals* GLOBAL){	
	int i = 0;
	//for each event, initialize the handler structure
	for( i = 0; i < EVENTS_NUM_EVENTS; i++){
		GLOBAL->EVENT[i].td = NULL;
		GLOBAL->EVENT[i].too_slow = 0;
		//initialize the character buffer
		cbuffer_init( &GLOBAL->EVENT[i].cbuff, GLOBAL->EVENT[i].buff_mem, EVENTS_CHAR_BUFFER_SIZE );
	}
}

void initialize_first_task(struct kern_globals* GLOBAL){

	struct task_descriptor* td = new_task(GLOBAL, &task_system_first, PRIORITY_SYSTEM_FIRST);
	//struct task_descriptor* td = new_task(GLOBAL, &task_test_time, 4);
	
	//add first task to the priority queue
	set_ready(GLOBAL, td);
}
