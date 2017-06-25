#include <setjmp.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct generator;

struct generator {
	void (*next)(struct generator*, void*);
	void* arg;

	jmp_buf generator_next_env;
	jmp_buf generator_yield_env;

	void* stack_beginning;
	long long unsigned int stack_size;
	void* stack_backup;

	void* result;

	int first_run;
	int initialize;
};

struct generator* generator_new (void (*fn)(struct generator*, void*), void* arg) {
	struct generator* c = calloc(sizeof(struct generator), 1);
	c->first_run = 1;
	c->arg = arg;
	c->next = fn;
	return c;
}

void* generator_next (struct generator* c) {
	char stack;
	c->stack_beginning = &stack;
	c->initialize = 1;
	if (!setjmp(c->generator_next_env))
		c->next(c, c->arg);
	return c->result;
}

void generator_yield (struct generator* c, void* result) {
	int stack;
	void* stack_end = &stack;

	if (!c->initialize || c->first_run) {
		c->first_run = 0;
		// stack_beginning - stack_end because the stack grows downward
		c->stack_size = c->stack_beginning - stack_end;

		// save the stack in the heap
		c->stack_backup = realloc(c->stack_backup, c->stack_size);
		memcpy(c->stack_backup, stack_end, c->stack_size);

		// if result is in the stack, make it point to its heap backup instead
		if (result < c->stack_beginning && result >= stack_end)
			c->result = c->stack_backup + (result - stack_end);
		else
			c->result = result;

		// save current stack pointer, code pointer and so on
		if (!setjmp(c->generator_yield_env))
			longjmp(c->generator_next_env, 1);
		c->initialize = 0;
		memcpy(c->stack_beginning - c->stack_size, c->stack_backup, c->stack_size);
		return;
	}
	longjmp(c->generator_yield_env, 1);
}

void fib (struct generator* c, void* arg) {
	int i = 1, j = 0, k;
	while (1) {
		generator_yield(c, &j);
		k = i;
		i += j;
		j = k;
	}
}

int main()
{
	struct generator* c1 = generator_new(fib, NULL);
	int* result1, i;
	for (i = 0; i < 10; ++i) {
		result1 = generator_next(c1);
		printf("%i: %i\n", i, *result1);
	}
	return 0;
}
