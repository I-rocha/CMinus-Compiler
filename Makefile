SYMTAB = symtab
EXE = exe
INPUT = input
EXAMPLE = example

all: lex.yy.o y.tab.o symtab.o ast.o semantic.o file_log.o
	gcc -o $(EXE) y.tab.c lex.yy.c symtab.o ast.o semantic.o file_log.o

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

semantic.o: semantic.c
	gcc -c semantic.c 

file.o: file_log.c
	gcc -c file_log.c

run1:	$(INPUT)/$(EXAMPLE)1.c
	./$(EXE) < $(INPUT)/$(EXAMPLE)1.c

run2:	$(INPUT)/$(EXAMPLE)2.c
	./$(EXE) < $(INPUT)/$(EXAMPLE)2.c
	
run3:	$(INPUT)/$(EXAMPLE)3.c
	./$(EXE) < $(INPUT)/$(EXAMPLE)3.c

clean:
	rm y.tab.* lex.yy.c 
	rm *.o 
	rm $(EXE)


