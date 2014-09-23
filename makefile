P = runGP
OBJECTS = gpparser.tab.o lex.yy.o ast.o rule.o pretty.o seman.o graph.o match.o staticsearch.o main.o
PARSEOBJECTS = gpparser.tab.o lex.yy.o ast.o seman.o pretty.o main.o  
CC = gcc
CFLAGS = -g -Wall -Wextra `pkg-config --cflags --libs glib-2.0`
#CFLAGS = -g -Wall -Wextra -I/local/d0p6/chrisbak/root/include/glib-2.0 -I/local/d0p6/chrisbak/root/lib/glib-2.0/include
LFLAGS = -lglib-2.0
VALGRIND = G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --tool=memcheck --leak-check=full --leak-resolution=high --track-origins=yes

# Builds executable runGP.
default:	$(OBJECTS)
		$(CC) $(OBJECTS) $(LFLAGS) -o $(P)

# Builds executable runGP and runs it on extensionless files.
# Usage: make runGP F1=<program_filename> F2=<graph_filename>
$(P):	        $(PARSEOBJECTS)
		$(CC) $(PARSEOBJECTS) $(LFLAGS) -o $(P) 	
		./$(P) $(F1) $(F2)       	

debug:		$(PARSEOBJECTS)
		$(CC) $(PARSEOBJECTS) $(LFLAGS) -o $(P) 	
		$(VALGRIND) --suppressions=GNOME.supp/glib.supp ./$(P) $(F1) $(F2)

# Testing file.
test:		ast.o graph.o match.o rule.o staticsearch.o test.o
		$(CC) ast.o graph.o match.o rule.o staticsearch.o test.o $(LFLAGS) -o testGP
		./testGP

test-debug:	ast.o graph.o match.o rule.o staticsearch.o test.o
		$(CC) ast.o graph.o match.o rule.o staticsearch.o test.o $(LFLAGS) -o testGP
		$(VALGRIND) --suppressions=GNOME.supp/glib.supp ./testGP



gpparser.tab.o: gpparser.tab.c gpparser.tab.h
		$(CC) $(CFLAGS) -c gpparser.tab.c

gpparser.tab.c gpparser.tab.h: gpparser.y ast.h
		bison -dtv gpparser.y

lex.yy.o: 	lex.yy.c 
		$(CC) $(CFLAGS) -c lex.yy.c

lex.yy.c:	gplexer.lex gpparser.tab.h ast.h 
		flex gplexer.lex

main.o:         main.c ast.h globals.h pretty.h seman.h
		$(CC) $(CFLAGS) -c main.c

ast.o: 		ast.c ast.h globals.h
		$(CC) $(CFLAGS) -c ast.c

rule.o:		rule.c globals.h graph.h rule.h
		$(CC) $(CFLAGS) -c rule.c

pretty.o:       pretty.c pretty.h ast.h globals.h seman.h
		$(CC) $(CFLAGS) -c pretty.c

seman.o:	seman.c seman.h ast.h globals.h
		$(CC) $(CFLAGS) -c seman.c

graph.o:	graph.c globals.h graph.h 
		$(CC) $(CFLAGS) -c graph.c

match.o:	match.c globals.h graph.h rule.h match.h 
		$(CC) $(CFLAGS) -c match.c

staticsearch.o:	staticsearch.c globals.h graph.h match.h rule.h staticsearch.h 
		$(CC) $(CFLAGS) -c staticsearch.c

test.o:		test.c staticsearch.h
		$(CC) $(CFLAGS) -c test.c
clean:
		rm *.o gpparser.tab.c gpparser.tab.h lex.yy.c runGP

