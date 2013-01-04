/* Header file for miscellaneous string functions */

#ifndef __STRINGLIB_H__
#define __STRINGLIB_H__

int safestrcpy( char * destination, char * source, int destsize );

int strlen( char* str );

int strcmp( const char * str1, const char * str2 );

char * strchr( const char *s, int ch );

int str_atoi( const char *s );

int str_is_integer( const char *s );

char char2hex( char ch );

#endif
