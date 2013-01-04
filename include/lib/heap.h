#ifndef __HEAP_H__
#define __HEAP_H__

struct heap {
	int size;
	int max_size;
	int * elements;
	int * priorities;
};


//Initialize heap
void heap_init(struct heap * h, int * elements, int * priorities, int max_size);
//Add a new element to the heap
void heap_add(struct heap * h, int element, int priority);
//Remove the elemen of minimum priority from the list
int heap_remove_min(struct heap *h);
//Check the pririty of the smallest elem
int heap_min_priority(struct heap *h);
//Print the eleme ntin the heap
void print_heap(struct heap *);

//Index of an element's parent in the heap
int heap_parent(int i);
//Index of an element's right child in the heap
int heap_left(int i);
//Index of an element's left child in the heap
int heap_right(int i);

/*
Example heap allocation:

struct heap task_heap;
int elements[CONFIG_CLK_MAX_DELAYED_TASKS], priorities[CONFIG_CLK_MAX_DELAYED_TASKS];
task_heap.size = 0;
task_heap.max_size = CONFIG_CLK_MAX_DELAYED_TASKS;
task_heap.elements = elements;
task_heap.priorities = priorities;
 */

#endif
