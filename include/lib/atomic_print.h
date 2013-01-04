/*
 * atmoic_print.h
 */

#ifndef __LIB_ATOMIC_PRINT__H__
#define __LIB_ATOMIC_PRINT__H__

#define AP_SUCCESS 0
#define AP_FAIL_BUFFER_FULL -1

#define ATOMIC_PRINT_BUFF_SIZE 1025
#define ATOMIC_PRINT_BUFF_LENGTH 1024

struct print_buffer{
	int length;
	char mem[ATOMIC_PRINT_BUFF_SIZE];
};

typedef char *ap_va_list;

#define __ap_va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define ap_va_start(argp, pN) ((argp) = ((ap_va_list) __builtin_next_arg(pN)))

#define ap_va_end(argp)	((void)0)

#define ap_va_arg(argp, t)	\
		 (((argp) = (argp) + __ap_va_argsiz(t)), *((t*) (void*) ((argp) - __ap_va_argsiz(t))))

//initalize the print buffer to defaults
void ap_init_buff( struct print_buffer* buff );

//put a character to the buffer
int ap_putc( struct print_buffer* buff, char c );

//put hex to the buffer
int ap_putx( struct print_buffer* buff, char c );

//put a string to the buffer
int ap_putstr( struct print_buffer* buff, char *str );

//print an integer as hex to the buffer
int ap_putr( struct print_buffer* buff, unsigned int reg );

//print with a fill character to the buffer
int ap_putw( struct print_buffer* buff, int n, char fc, char *bf );

//printf to the buffer
void ap_printf( struct print_buffer* buff, char *fmt, ... );

void ap_format ( struct print_buffer* buff, char *fmt, ap_va_list va );


#endif
