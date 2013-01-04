#include <commonspace.h>

void heap_init(struct heap * h, int * elements, int * priorities, int max_size) {
	h->elements = elements;
	h->priorities = priorities;
	h->size = 0;
	h->max_size = max_size;
}

int heap_parent(int i) {
	return (i + 1) / 2 - 1;
}

int heap_left(int i) {
	return i * 2 + 1;
}

int heap_right(int i) {
	return i * 2 + 2;
}

void heap_swap(struct heap * h, int i, int j) {
	int temp1 = 0, temp2 = 0;
	temp1 = h->elements[i];
	temp2 = h->priorities[i];
	h->elements[i] = h->elements[j];
	h->priorities[i] = h->priorities[j];
	h->elements[j] = temp1;
	h->priorities[j] = temp2;
}

void heap_add(struct heap * h, int element, int priority)  {
	int i = h->size;
	kernel_assert(h->size < h->max_size, h->size < h->max_size)
	h->elements[h->size] = element;
	h->priorities[h->size] = priority;
	h->size++;
	while(i > 0 && h->priorities[heap_parent(i)] > h->priorities[i]) {
		heap_swap(h, i, heap_parent(i));
		i = heap_parent(i);
	}
}

int heap_remove_min(struct heap *h) {
	int min = 0, i = 0;
	if(h->size > 0) {
		min = h->elements[0];
		h->elements[0] = h->elements[h->size -1];
		h->priorities[0] = h->priorities[h->size -1];
		h->size--;
		while(i < h->size) {
			if(heap_left(i) < h->size && h->priorities[heap_left(i)] < h->priorities[i] && h->priorities[heap_left(i)] <= h->priorities[heap_right(i)]) {
				heap_swap(h, i, heap_left(i));
				i = heap_left(i);
			} else if(heap_right(i) < h->size && h->priorities[heap_right(i)] < h->priorities[i] && h->priorities[heap_right(i)] <= h->priorities[heap_left(i)]) {
				heap_swap(h, i, heap_right(i));
				i = heap_right(i);
			} else {
				break;
			}
		}
	}
	return min;
}

void print_heap(struct heap * h) {
	int i = 0;
	bwprintf(COM2, "HEAP: ");
	for(i = 0; i < h->size; i++) {
		bwprintf(COM2, " [%d, %d] ",h->priorities[i], h->elements[i]);
	}
	bwprintf(COM2, "\r\n");
}


int heap_min_priority(struct heap *h) {
	if(h->size > 0) {
		return h->priorities[0];
	} else {
		return -1;
	}
}

