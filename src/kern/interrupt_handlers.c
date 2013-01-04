/**
 *
 * interrupt_handlers.c
 *
 **/
 
#include <kernspace.h>


void handle_interrupt_clock(struct kern_globals* GLOBAL);
void handle_interrupt_uart1(struct kern_globals* GLOBAL);
void handle_interrupt_uart2(struct kern_globals* GLOBAL);

void handle_interrupts(struct kern_globals* GLOBAL){
	//check each interrupt bit, and handle it if high
	
	//get the interrupt status register
	int* vic2 = (int*)VIC2_BASE;	
	
	//while IRQ high, handle the interrupts
	while(*(vic2 + VIC_IRQSTATUS)){
		//timer 3
		if( *(vic2 + VIC_IRQSTATUS) & VIC2_TIMER_MASK ){ handle_interrupt_clock(GLOBAL); }
	
		//UART 1
		if( *(vic2 + VIC_IRQSTATUS) & VIC2_UART1_MASK ){ handle_interrupt_uart1(GLOBAL); }
	
		//UART 2
		if( *(vic2 + VIC_IRQSTATUS) & VIC2_UART2_MASK ){ handle_interrupt_uart2(GLOBAL); }
	}
}


void handle_interrupt_clock(struct kern_globals* GLOBAL){
			//Clear The Timer
			unsigned int* t3 = (unsigned int*) TIMER3_BASE;	//get the timer address
			//Clear the interrupt
			t3[3] = 1;		
			
			cbuffer_push_char( &(GLOBAL->EVENT[EVENT_CLOCK_TICK].cbuff), 'A');
			
			if(GLOBAL->EVENT[EVENT_CLOCK_TICK].td != NULL){
				struct request_await_event* req = (struct request_await_event*) GLOBAL->EVENT[EVENT_CLOCK_TICK].td->req;
			
				//set the return value
				req->retval = 0;

				int i = 0;
				while ( GLOBAL->EVENT[EVENT_CLOCK_TICK].cbuff.state != CBUFF_EMPTY && i < 15){
					req->event[i] = cbuffer_pop( &(GLOBAL->EVENT[EVENT_CLOCK_TICK].cbuff));
					i++;
				}
				req->event[i] = '\0';
			
				//ready the task
				set_ready(GLOBAL, GLOBAL->EVENT[EVENT_CLOCK_TICK].td);
			
				//clear the event slot
				GLOBAL->EVENT[EVENT_CLOCK_TICK].td = NULL;
			}
}

void handle_interrupt_uart1(struct kern_globals* GLOBAL){
	//handle the UART1 (TERMINAL) Interrupt
	
	int uart = UART1_BASE;
	
	//bwputc(COM2, '1');
	
	//check if we have something to receive
	//must also get the timeout interrupt (see EP9301 User Guide pg 383)
	if( *((int *)(uart + UART_INTR_OFFSET)) & 0x2){
		//bwputc(COM2, '2');
	
		//get the character (which clears the interrupt)
		char c = *((int *)(uart + UART_DATA_OFFSET));
		
		//inform the waiting event
		if(GLOBAL->EVENT[EVENT_UART1_RECEIVE_READY].td != NULL){
			struct request_await_event* req = (struct request_await_event*) GLOBAL->EVENT[EVENT_UART1_RECEIVE_READY].td->req;
			
			//set the return value
			req->retval = 0;
			
			req->event[0] = (char) (c & 0xFF);
			
			//ready the task
			set_ready(GLOBAL, GLOBAL->EVENT[EVENT_UART1_RECEIVE_READY].td);
			
			//clear the event slot
			GLOBAL->EVENT[EVENT_UART1_RECEIVE_READY].td = NULL;
		} else {
			//we should queue the event here, should be modified.
			bwputc(COM2,'&');
		}
		
	} else if(*((int *)(uart + UART_INTR_OFFSET)) & 0x4 || *((int *)(uart + UART_INTR_OFFSET)) & 0x1){		
		//bwputc(COM2, '3');
	
		//inform the waiting event
		if(GLOBAL->EVENT[EVENT_UART1_SEND_READY_INITIAL].td != NULL){
			struct request_await_event* req = (struct request_await_event*) GLOBAL->EVENT[EVENT_UART1_SEND_READY_INITIAL].td->req;
			
			//set the return value
			req->retval = 0;
						
			//ready the task
			set_ready(GLOBAL, GLOBAL->EVENT[EVENT_UART1_SEND_READY_INITIAL].td);
			
			//clear the event slot
			GLOBAL->EVENT[EVENT_UART1_SEND_READY_INITIAL].td = NULL;		
			
		}

		//inform the waiting event
		if(GLOBAL->EVENT[EVENT_UART1_SEND_READY].td != NULL){
			struct request_await_event* req = (struct request_await_event*) GLOBAL->EVENT[EVENT_UART1_SEND_READY].td->req;
			
			//set the return value
			req->retval = 0;
						
			//ready the task
			set_ready(GLOBAL, GLOBAL->EVENT[EVENT_UART1_SEND_READY].td);
			
			//clear the event slot
			GLOBAL->EVENT[EVENT_UART1_SEND_READY].td = NULL;		
			
		} else {
			//Ignore this! We don't have anything to send.
		}
		
		if(*((int *)(uart + UART_INTR_OFFSET)) & 0x4) {
			//Deactivate the receive interrupt
			int * ctlr, buf;
			ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
			buf = *ctlr;
			*ctlr = buf & ~TIEN_MASK;						
		}
		
		if(*((int *)(uart + UART_INTR_OFFSET)) & 0x1) {
			//clear modem status interrupt
			*((int *)(uart + UART_INTR_OFFSET)) = 0;			
			
			//Deactivate the modem status interrupt
			int * ctlr, buf;
			ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
			buf = *ctlr;
			*ctlr = buf & ~MSIEN_MASK;				

		}
		
	} else {
		bwputc(COM2,'_');
	}
	
	
}

void handle_interrupt_uart2(struct kern_globals* GLOBAL){
	//handle the UART2 (TERMINAL) Interrupt
	
	int uart = UART2_BASE;
	
	//check if we have something to receive
	//TODO: NEED TO CHECK FOR RECEIVE TIMEOUT, AND RECEIVE INTERRUPT (pg 383, EP3901 Guide)
	if( *((int *)(uart + UART_INTR_OFFSET)) & 0x2 || *((int *)(uart + UART_INTR_OFFSET)) & 0x8){
		
		//buffer the uart characters
		
		char c;
		
		while ( !(*((int *)(uart + UART_FLAG_OFFSET)) & RXFE_MASK)){
			c = *((int *)(uart + UART_DATA_OFFSET));
			cbuffer_push_char( &(GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].cbuff), c);
		}
		
		//inform the waiting event
		if(GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].td != NULL){
			struct request_await_event* req = (struct request_await_event*) GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].td->req;
			
			//set the return value
			req->retval = 0;

			int i = 0;
			while ( GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].cbuff.state != CBUFF_EMPTY && i < 15){
				req->event[i] = cbuffer_pop( &(GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].cbuff));
				i++;
			}
			req->event[i] = '\0';
			
			//ready the task
			set_ready(GLOBAL, GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].td);
			
			//clear the event slot
			GLOBAL->EVENT[EVENT_UART2_RECEIVE_READY].td = NULL;
		} else {
			//we have buffered the characters
			//bwputc(COM2,'@');
		}
		
		
	} else if(*((int *)(uart + UART_INTR_OFFSET)) & 0x4){		
		//inform the waiting event
		if(GLOBAL->EVENT[EVENT_UART2_SEND_READY].td != NULL){
			struct request_await_event* req = (struct request_await_event*) GLOBAL->EVENT[EVENT_UART2_SEND_READY].td->req;
			
			//set the return value
			req->retval = 0;
						
			//ready the task
			set_ready(GLOBAL, GLOBAL->EVENT[EVENT_UART2_SEND_READY].td);
			
			//clear the event slot
			GLOBAL->EVENT[EVENT_UART2_SEND_READY].td = NULL;		
			
		} else {
			//Ignore this! We don't have anything to send.
		}
		
		//Deactivate the receive interrupt
		int * ctlr, buf;
		ctlr = (int *)( UART2_BASE + UART_CTLR_OFFSET );
		buf = *ctlr;
		*ctlr = buf & ~ TIEN_MASK;			
		
	} else {
		bwputc(COM2,'_');
	}
	
	//clear modem status interrupt
	*((int *)(uart + UART_INTR_OFFSET)) = 0;
}

