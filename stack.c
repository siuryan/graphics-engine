#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"
#include "stack.h"

/*======== struct stack * new_stack()) ==========
  Inputs:   
  Returns: 
  
  Creates a new stack and puts an identity
  matrix at the top.
  ====================*/
struct stack * new_stack() {

  struct stack *s;
  struct matrix **m;
  struct matrix *i;
  s = (struct stack *)malloc(sizeof(struct stack));
  
  m = (struct matrix **)malloc( STACK_SIZE * sizeof(struct matrix *));
  i = new_matrix(4, 4);
  ident( i );

  s->size = STACK_SIZE;
  s->top = 0;
  s->data = m;
  s->data[ s->top ] = i;

  return s;
}

/*======== struct matix *peek() ==========
  Inputs:   struct stack *s  
  Returns: 

  Returns a reference to the matrix at the 
  top of the stack
  ====================*/
struct matrix * peek( struct stack *s ) {
  return s->data[s->top];
}

/*======== void push() ==========
  Inputs:   struct stack *s  
  Returns: 

  Puts a new matrix on top of s
  The new matrix should be a copy of the curent
  top matrix
  ====================*/
void push( struct stack *s ) {

  struct matrix *m;
  m = new_matrix(4, 4);
  
  if ( s->top == s->size - 1 ) {
    s->data = (struct matrix **)realloc( s->data, (s->size + STACK_SIZE)
					 * sizeof(struct matrix *));
    s->size = s->size + STACK_SIZE;
  }

  copy_matrix( s->data[ s->top ], m);

  s->top++;
  s->data[ s->top ] = m;  
}

/*======== void pop() ==========
  Inputs:   struct stack * s 
  Returns: 
  
  Remove and free the matrix at the top
  Note you do not need to return anything.
  ====================*/
void pop( struct stack * s) {

  free_matrix(s->data[s->top]);
  s->top--;
}

/*======== void free_stack() ==========
  Inputs:   struct stack *s 
  Returns: 

  Deallocate all the memory used in the stack
  ====================*/
void free_stack( struct stack *s) {

  int i;
  for (i=0; i <= s->top; i++) {

    free_matrix(s->data[i]);
  }
  free(s->data);
  free(s);
}

void print_stack(struct stack *s) {

  int i;
  for (i=s->top; i >= 0; i--) {

    print_matrix(s->data[i]);
    printf("\n");
  }

}
