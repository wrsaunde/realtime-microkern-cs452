/*
 * int_buffer.h
 */

#ifndef __LIB_INT_BUFFER__H__
#define __LIB_INT_BUFFER__H__

#define INTBUFF_EMPTY 1
#define INTBUFF_NOT_EMPTY 0
#define INTBUFF_FULL 2

struct int_buffer {
	unsigned int state;				//empty state of charbuffer

	unsigned int size;				//size of the buffer
	int* storage;					//pointer to buffer storage space

	unsigned int write_position;	//current place to write
	unsigned int read_position;     //current place to read
};

void intbuffer_init( struct int_buffer* intbuffer, int* storage_array, unsigned int storage_size );
int intbuffer_push( struct int_buffer* intbuffer, int i );
int intbuffer_pop( struct int_buffer* intbuffer );


#endif
