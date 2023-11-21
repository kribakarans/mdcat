#ifndef __MDCAT_STACK_H__
#define __MDCAT_STACK_H__

#define STACK_SIZE 10

enum {
	STACK_OK    =  0,
	STACK_ERR   = -1,
	STACK_FULL  = -2,
	STACK_EMPTY = -3,
};

/* Stack structure */
typedef struct stack_st {
	int top;
	int element[STACK_SIZE];
} stack_t;

int initialize_stack(stack_t *stack);
int push(stack_t *stack, int value);
int pop(stack_t *stack);
int peek(stack_t *stack);
int display_stack(stack_t *stack);

#endif
