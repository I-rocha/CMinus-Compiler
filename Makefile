main: parser.tab.o lex.yy.o ast.o main.o utils.o symtab.o semantic.o
	gcc parser.tab.o lex.yy.o ast.o main.o symtab.o semantic.o -o exe	

lex.yy.o: lex.yy.c
	gcc lex.yy.c -Wall -c

lex.yy.c: words.l
	flex words.l

parser.tab.o: parser.tab.c
	gcc parser.tab.c -Wall -c

parser.tab.c: parser.y
	bison -d parser.y

main.o: main.c
	gcc main.c -Wall -c

utils.o: utils.c
	gcc utils.c -Wall -c

symtab.o: lexical/symtab.c
	gcc lexical/symtab.c -Wall -c

semantic.o: semantic/semantic.c
	gcc semantic/semantic.c -Wall -c

ast.o: ast/ast.c
	gcc ast/ast.c -Wall -c

clean:
	rm -f parser.tab.* lex.yy.c *.o
