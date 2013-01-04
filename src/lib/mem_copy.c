#include <lib/mem_copy.h>

void mem_copy(char* source, char* target, int num_bytes){
	int * src = (int*) source;
	int * tar = (int*) target;
	//copy as far as possible entire words
	while(num_bytes >= 4){
		*(tar++) = *(src++);
		num_bytes -= 4;
	}

	target = (char*) tar;
	source = (char*) src;
	
	//copy the non-word aligned end
	while(num_bytes > 0){
		*(target++) = *(source++);
		num_bytes--;
	}
}
