#include <userspace.h>

void task_echo_train_input() {
	char c = 42;
	int putbuffret;
	struct print_buffer pbuff;
	
	while(1) {
		c = Getc(COM1);
		ap_init_buff( &pbuff );
		ap_printf( &pbuff, "Sensor Input:");
		ap_putr(&pbuff, c);
		ap_printf( &pbuff, "\n");
		putbuffret = Putbuff( COM2, &pbuff );		
	}
}

void task_test_train() {
	int putbuffret, i = 0;
	int status;

Create(2, &task_echo_train_input);	
	

	
	//send an initial message
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );
	ap_printf( &pbuff, "START!\n");
	putbuffret = Putbuff( COM2, &pbuff );

	Putc(COM1, 96);		
	
	for(i = 0; i < 100; i++) {
		status = SetTrainSpeed(i, 10);
		ap_init_buff( &pbuff );
		ap_printf( &pbuff, "status:%d\n",status);
		putbuffret = Putbuff( COM2, &pbuff );
	}

	ap_init_buff( &pbuff );
	ap_printf( &pbuff, "SENSOR QUERY!\n");
	putbuffret = Putbuff( COM2, &pbuff );


	for(i = 0; i < 10; i++) {
		Putc(COM1, 133);
		Delay(30);
	}	

	ap_init_buff( &pbuff );
	ap_printf( &pbuff, "STOP!\n");
	putbuffret = Putbuff( COM2, &pbuff );
	
	for(i = 0; i < 100; i++) {
		SetTrainSpeed(i, 0);
	}

	ap_init_buff( &pbuff );
	ap_printf( &pbuff, "SENSOR POLL!\n");
	putbuffret = Putbuff( COM2, &pbuff );

	while(1) {
		Putc(COM1, 133);
		Delay(100);
	}

}

