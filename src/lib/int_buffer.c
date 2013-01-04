/*
	int_buffer.c - A circulur int queue
	
*/

#include <lib/all.h>

void intbuffer_init( struct int_buffer* intbuffer, int* storage_array, unsigned int storage_size ){
        intbuffer->storage = storage_array;
        intbuffer->size = storage_size;
        intbuffer->state = INTBUFF_EMPTY;
        intbuffer->write_position = 0;
        intbuffer->read_position = 0;
}
int intbuffer_push( struct int_buffer* intbuffer, int i ){
	if(intbuffer->state == INTBUFF_FULL){ return -1; }
	
	intbuffer->storage[intbuffer->write_position] = i;
	intbuffer->write_position = (intbuffer->write_position + 1) % intbuffer->size;
	
	if(intbuffer->write_position == intbuffer->read_position){
		intbuffer->state = INTBUFF_FULL;
	}else{
		intbuffer->state = INTBUFF_NOT_EMPTY;
	}
	
	return 0;
}

int intbuffer_pop( struct int_buffer* intbuffer ){
	int ret = intbuffer->storage[intbuffer->read_position];
	intbuffer->read_position = (intbuffer->read_position + 1) % intbuffer->size;
	if(intbuffer->read_position == intbuffer->write_position){
		intbuffer->state = INTBUFF_EMPTY;
	}else{
		intbuffer->state = INTBUFF_NOT_EMPTY;
	}
	return ret;
}
