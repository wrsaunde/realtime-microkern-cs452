#include <lib/all.h>
#include <config/key_words.h>


int safestrcpy( char * destination, char * source, int destsize ) {
	int i = 0, copied = 0;
	while( i < destsize - 1 && source[i] != '\0' ) {
		destination[i] = source[i];
		i++;
	}
	copied = i;
	while( i < destsize ) {
		destination[i] = '\0';
		i++;
		break;
	}
	return copied;
}


int strlen( char* str ) {
	int i = 0;
	while( *str++ != '\0' ) {
		i++;
	}
	return i;
}


int strcmp( const char * str1, const char * str2 ) {
	while( *str1 == *str2 && *str1 != '\0' ) {
		str1++;
		str2++;
	}
	if( *str1 == '\0' && *str2 == '\0' ) {
		return 0;
	}
	return *str1 - *str2;
}


char char2hex( char ch ) {
	if( (ch <= 9) ) return '0' + ch;
	return 'a' + ch - 10;
}


//check if a string is an integer


int str_is_integer( const char *s ) {
	int i = 0;
	if( s[0] == '\0' ) {
		return FALSE;
	}
	if( s[0] == '-' ) {
		if( s[1] == '\0' ) {
			return FALSE;
		}
		i++;
	}

	while( s[i] != '\0' ) {
		if( s[i] < '0' || s[i] > '9' ) {
			return FALSE;
		}
		i++;
	}

	return TRUE;
}


int str_atoi( const char *s ) {
	/*
	 * This code has been removed while open sourcing due to licensing.
	 * Any standard implementation of this function should work in its
	 * place.
	 */
	return 0;
}


char *
strchr( const char *s, int ch ) {
	/*
	 * This code has been removed while open sourcing due to licensing.
	 * Any standard implementation of this function should work in its
	 * place.
	 */
	return (char*) 0;
}
