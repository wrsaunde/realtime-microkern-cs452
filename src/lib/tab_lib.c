#include <userspace.h>

void clear_tab_space( ) {
	struct print_buffer pbuff;
	ap_init_buff( &pbuff );

	/* draw the outlines */

	ap_putstr( &pbuff, "\x1B[?25l" ); //turn the cursor off
	ap_putstr( &pbuff, "\x1B[33m" ); //set attributes

	//clear the lines and redraw the ends
	int i = 0;
	for( i = 5; i <= 16; i++ ) {
		ap_printf( &pbuff, "\x1B[%d;24f\x1B[K\x1B[%d;79f#", i, i );
	}

	ap_putstr( &pbuff, "\x1B[0m" ); //clear attributes
	ap_putstr( &pbuff, "\x1B[u\x1B[?25h" ); //reload cursor position and enable

	Putbuff( COM2, &pbuff );
}