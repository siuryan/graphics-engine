OBJECTS= symtab.o print_pcode.o matrix.o my_main.o display.o draw.o gmath.o stack.o mesh.o
CFLAGS= -g
LDFLAGS= -lm
CC= gcc

all: parser
	./mdl cow.mdl

parser: lex.yy.c y.tab.c y.tab.h $(OBJECTS)
	gcc -o mdl $(CFLAGS) lex.yy.c y.tab.c $(OBJECTS) $(LDFLAGS)

lex.yy.c: mdl.l y.tab.h 
	flex -I mdl.l

y.tab.c: mdl.y symtab.h parser.h
	bison -d -y mdl.y

y.tab.h: mdl.y 
	bison -d -y mdl.y

symtab.o: symtab.c parser.h matrix.h
	gcc -c $(CFLAGS) symtab.c

print_pcode.o: print_pcode.c parser.h matrix.h
	gcc -c $(CFLAGS) print_pcode.c

matrix.o: matrix.c matrix.h
	gcc -c $(CFLAGS) matrix.c

my_main.o: my_main.c parser.h print_pcode.c matrix.h display.h ml6.h draw.h stack.h
	gcc -c $(CFLAGS) my_main.c

display.o: display.c display.h ml6.h matrix.h
	$(CC) $(CFLAGS) -c display.c

draw.o: draw.c draw.h display.h ml6.h matrix.h gmath.h
	$(CC) $(CFLAGS) -c draw.c

gmath.o: gmath.c gmath.h matrix.h
	$(CC) $(CFLAGS) -c gmath.c

stack.o: stack.c stack.h matrix.h
	$(CC) $(CFLAGS) -c stack.c

mesh.o: mesh.c mesh.h
	$(CC) $(CFLAGS) -c mesh.c

clean:
	rm y.tab.c y.tab.h
	rm lex.yy.c
	rm -rf mdl.dSYM
	rm *.o *~
