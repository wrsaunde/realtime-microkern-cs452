/*
	char_buffer.c - A circulur char queue
	
*/

#include <lib/all.h>

void cbuffer_init( struct char_buffer* cbuffer, char* storage_array, unsigned int storage_size ){
	cbuffer->storage = storage_array;
	cbuffer->size = storage_size;
	cbuffer->state = CBUFF_EMPTY;
	cbuffer->write_position = 0;
	cbuffer->read_position = 0;
}

void cbuffer_empty( struct char_buffer* cbuffer){
	cbuffer->state = CBUFF_EMPTY;
	cbuffer->write_position = 0;
	cbuffer->read_position = 0;
}
//Add a character to the buffer
int cbuffer_push_char( struct char_buffer* cbuffer, char character ){
	if(cbuffer->state == CBUFF_FULL){ return -1; }
	
	cbuffer->storage[cbuffer->write_position] = character;
	cbuffer->write_position = (cbuffer->write_position + 1) % cbuffer->size;
	
	if(cbuffer->write_position == cbuffer->read_position){
		cbuffer->state = CBUFF_FULL;
	}else{
		cbuffer->state = CBUFF_NOT_EMPTY;
	}
	
	return 0;
}

//copy a string to the buffer
int cbuffer_push_string( struct char_buffer* cbuffer, char* str, int max_length){
	while(max_length-- && *str){
		int code = cbuffer_push_char( cbuffer, *(str++) );
		if(code < 0){ return -1; }
	}
	return 0;
}

//Get the next character from the buffer, should make sure buffer is not empty in other code (FIFO)
char cbuffer_pop( struct char_buffer* cbuffer ){
	char ret = cbuffer->storage[cbuffer->read_position];
	cbuffer->read_position = (cbuffer->read_position + 1) % cbuffer->size;
	if(cbuffer->read_position == cbuffer->write_position){
		cbuffer->state = CBUFF_EMPTY;
	}else{
		cbuffer->state = CBUFF_NOT_EMPTY;
	}
	return ret;
}

char cbuffer_peek( struct char_buffer* cbuffer ){
	char ret = cbuffer->storage[cbuffer->read_position];
	return ret;
}

//Remove the last character from the buffer, (FILO)
char cbuffer_pop_last( struct char_buffer* cbuffer ){
	cbuffer->write_position = cbuffer->write_position - 1;
	if(cbuffer->write_position == -1){ cbuffer->write_position = cbuffer->size - 1; }
	char ret = cbuffer->storage[cbuffer->write_position];
	if(cbuffer->read_position == cbuffer->write_position){
		cbuffer->state = CBUFF_EMPTY;
	}else{
		cbuffer->state = CBUFF_NOT_EMPTY;
	}
	return ret;
}

unsigned int cbuffer_size( struct char_buffer* cbuffer ){
	if(cbuffer->state == CBUFF_EMPTY){ return 0; }
	if(cbuffer->state == CBUFF_FULL ){ return cbuffer->size; }
	
	if(cbuffer->write_position > cbuffer->read_position){
		return cbuffer->write_position - cbuffer->read_position;	
	}else{
		return cbuffer->size - (cbuffer->read_position - cbuffer->write_position);
	}
}
