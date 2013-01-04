/**
 *
 * printf.c
 *
 **/

#include <userspace.h>

void Printf( int channel, char *fmt, ... ) {
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );
	
	ap_va_list va;

	ap_va_start(va,fmt);
	ap_format( &pbuff, fmt, va );
	ap_va_end(va);
	
	Putbuff(channel, &pbuff);
}

void CommandOutput( char *fmt, ... ) {
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );
	
	//move to the output buffer
	ap_putstr(&pbuff, "\x1B[?25l\x1B[23;79f\n");
	
	ap_va_list va;

	ap_va_start(va,fmt);
	ap_format( &pbuff, fmt, va );
	ap_va_end(va);
	
	ap_putstr(&pbuff, "\x1B[u\x1B[?25h");
	
	Putbuff(COM2, &pbuff);
}
