
#include <stdio.h>
#include "stack.h"

/* Check if the stack is empty */
int is_stack_empty(stack_t *stack)
{
	return stack->top == -1;
}

/* Check if the stack is full */
int is_stack_full(stack_t *stack)
{
	return stack->top == STACK_SIZE - 1;
}

/* Initialize the stack */
int initialize_stack(stack_t *stack)
{
	int retval = -1;

	do {
		if (stack == NULL) {
			retval = STACK_ERR;
			break;
		} else {
			stack->top = -1;
			retval = STACK_OK;
			break;
		}
	} while(0);

	return retval;
}

/* Push an element onto the stack */
int push(stack_t *stack, int value)
{
	int retval = -1;

	do {
		if (is_stack_full(stack)) {
			retval = STACK_FULL;
			break;
		} else {
			stack->element[++stack->top] = value;
			retval = STACK_OK;
			break;
		}
	} while(0);

	return retval;
}

/* Pop an element from the stack */
int pop(stack_t *stack)
{
	int retval = -1;

	do {
		if (is_stack_empty(stack)) {
			retval = STACK_EMPTY;
			break;
		} else {
			retval = stack->element[stack->top--];
			break;
		}
	} while(0);

	return retval;
}

/* Get the top element of the stack without removing it */
int peek(stack_t *stack)
{
	int retval = -1;

	do {
		if (is_stack_empty(stack)) {
			retval = STACK_EMPTY;
			break;
		} else {
			retval = stack->element[stack->top];
			break;
		}
	} while(0);

	return retval;
}

/* Display the elements of the stack */
int display_stack(stack_t *stack)
{
	int i = 0;
	int retval = -1;

	do {
		if (is_stack_empty(stack)) {
			retval = STACK_EMPTY;
			break;
		} else {
			printf("Stack elements: ");

			for (i = 0; i <= stack->top; i++) {
				printf("%d ", stack->element[i]);
			}

			printf("\n");
			retval = STACK_OK;
			break;
		}
	} while(0);

	return retval;
}

/* EOF */
