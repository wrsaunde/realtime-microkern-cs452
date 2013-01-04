/*
 * char_buffer.h
 */

#ifndef __LIB_CHAR_BUFFER__H__
#define __LIB_CHAR_BUFFER__H__

#define CBUFF_EMPTY 1
#define CBUFF_NOT_EMPTY 0
#define CBUFF_FULL 2

struct char_buffer {
	unsigned int state;				//empty state of charbuffer

	unsigned int size;				//size of the buffer
	char* storage;					//pointer to buffer storage space

	unsigned int write_position;	//current place to write
	unsigned int read_position;     //current place to read
};

void cbuffer_init( struct char_buffer* cbuffer, char* storage_array, unsigned int storage_size );
void cbuffer_empty( struct char_buffer* cbuffer);
int cbuffer_push_char( struct char_buffer* cbuffer, char character );
int cbuffer_push_string( struct char_buffer* cbuffer, char* str, int max_length);
char cbuffer_pop( struct char_buffer* cbuffer );
char cbuffer_peek( struct char_buffer* cbuffer );
char cbuffer_pop_last( struct char_buffer* cbuffer );
unsigned int cbuffer_size( struct char_buffer* cbuffer );


#endif
