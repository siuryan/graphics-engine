#ifndef STACK_H
#define STACK_H

#define STACK_SIZE 2

struct stack {
  int size;
  int top;
  struct matrix **data;
};

struct stack * new_stack();
struct matrix * peek( struct stack *s );
void push( struct stack *s );
void pop(struct stack *s);

void free_stack( struct stack *);
void print_stack( struct stack *);

#endif
