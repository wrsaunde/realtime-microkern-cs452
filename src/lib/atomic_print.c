/*
	atomic_print.c - busy-wait I/O routines for diagnosis
	
*/

#include <lib/all.h>

//private functions
char ap_a2i( char ch, char **src, int base, int *nump );
int ap_a2d( char ch );
void ap_ui2a( unsigned int num, unsigned int base, char *bf );
void ap_i2a( int num, char *bf );
//end of privates

void ap_init_buff( struct print_buffer* buff ){
	buff->length = 0;
	buff->mem[0] = '\0';
}

int ap_putc( struct print_buffer* buff, char c ){
	if(buff->length >= ATOMIC_PRINT_BUFF_LENGTH){
		return AP_FAIL_BUFFER_FULL;
	}
	buff->mem[buff->length++] = c;
	buff->mem[buff->length] = '\0';
	return AP_SUCCESS;
}

int ap_putx( struct print_buffer* buff, char c ) {
	char chh, chl;

	chh = char2hex( c / 16 );
	chl = char2hex( c % 16 );
	ap_putc( buff, chh );
	return ap_putc( buff, chl );
}

int ap_putstr( struct print_buffer* buff, char *str ) {
	while( *str ) {
		int ret_val = ap_putc( buff, *str );
		if( ret_val < 0 ){
			return ret_val;
		}
		str++;
	}
	return AP_SUCCESS;
}

int ap_putr( struct print_buffer* buff, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) ap_putx( buff, ch[byte] );
	return ap_putc( buff, ' ' );
}

int ap_putw( struct print_buffer* buff, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;
	int ret_val = AP_SUCCESS;
	
	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) ret_val = ap_putc( buff, fc );
	while( ( ch = *bf++ ) ) ret_val = ap_putc( buff, ch );
	
	return ret_val;
}

void ap_printf( struct print_buffer* buff, char *fmt, ... ) {
        ap_va_list va;

        ap_va_start(va,fmt);
        ap_format( buff, fmt, va );
        ap_va_end(va);
}

//start of private function definitions

void ap_format ( struct print_buffer* buff, char *fmt, ap_va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			ap_putc( buff, ch );
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = ap_a2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				ap_putc( buff, ap_va_arg( va, char ) );
				break;
			case 's':
				ap_putw( buff, w, 0, ap_va_arg( va, char* ) );
				break;
			case 'u':
				ap_ui2a( ap_va_arg( va, unsigned int ), 10, bf );
				ap_putw( buff, w, lz, bf );
				break;
			case 'd':
				ap_i2a( ap_va_arg( va, int ), bf );
				ap_putw( buff, w, lz, bf );
				break;
			case 'x':
				ap_ui2a( ap_va_arg( va, unsigned int ), 16, bf );
				ap_putw( buff, w, lz, bf );
				break;
			case '%':
				ap_putc( buff, ch );
				break;
			}
		}
	}
}

char ap_a2i( char ch, char **src, int base, int *nump ) {
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = ap_a2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

int ap_a2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

void ap_ui2a( unsigned int num, unsigned int base, char *bf ) {
	int n = 0;
	int dgt;
	unsigned int d = 1;
	
	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

void ap_i2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	ap_ui2a( num, 10, bf );
}

