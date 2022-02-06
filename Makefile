SYMTAB = symtab
EXE = exe

all: lex.yy.o y.tab.o symtab.o ast.o
	gcc -o $(EXE) y.tab.c lex.yy.c symtab.o ast.o

lex.yy.o: lex.yy.c y.tab.h
	gcc -c lex.yy.c

lex.yy.c: words.l
	flex words.l

y.tab.o: y.tab.c
	gcc -c y.tab.c

y.tab.c y.tab.h: parser.y
	bison -dy parser.y

symtab.o: $(SYMTAB)/symtab.c
	gcc -c $(SYMTAB)/symtab.c

ast.o: ast.c
	gcc -c ast.c

run:
	gcc -o $(EXE) y.tab.c lex.yy.c
	./$(EXE)
	

clean:
	rm y.tab.* lex.yy.c 
	rm *.o 
	rm $(EXE)
