P = runGP
OBJECTS = gpparser.tab.o lex.yy.o ast.o structures.o pretty.o seman.o graph.o match.o main.o
CC = gcc
CFLAGS = -g -Wall -Wextra `pkg-config --cflags --libs glib-2.0`
#CFLAGS = -g -Wall -Wextra -I/local/d0p6/chrisbak/root/include/glib-2.0 -I/local/d0p6/chrisbak/root/lib/glib-2.0/include
LFLAGS = -lglib-2.0


# Builds executable gpparse and runs it on extensionless files.
# Usage: make F1=<program_filename> F2=<graph_filename>
default:        $(OBJECTS)
		$(CC) $(OBJECTS) $(LFLAGS) -o $(P) 	
		./$(P) $(F1) $(F2)       	

debug:		$(OBJECTS)
		$(CC) $(OBJECTS) $(LFLAGS) -o $(P) 	
		G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --tool=memcheck --leak-check=full --leak-resolution=high --track-origins=yes --suppressions=GNOME.supp/glib.supp ./$(P) $(F1) $(F2)

# For testing the graph API.
graph:		ast.o graph.o testgraph.o
		$(CC) graph.o testgraph.o $(LFLAGS) -o testgraph
		./testgraph

graph-debug:	ast.o graph.o testgraph.o
		$(CC) graph.o testgraph.o $(LFLAGS) -o testgraph
		G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --tool=memcheck --leak-check=full --leak-resolution=high --track-origins=yes ./testgraph



# Builds executable gpparse.
# Usage: make gpparse
$(P):		$(OBJECTS)
		$(CC) $(OBJECTS) $(LFLAGS) -o $(P)

gpparser.tab.o: gpparser.tab.c gpparser.tab.h
		$(CC) $(CFLAGS) -c gpparser.tab.c

gpparser.tab.c gpparser.tab.h: gpparser.y ast.h
		bison -dtv gpparser.y

lex.yy.o: 	lex.yy.c 
		$(CC) $(CFLAGS) -c lex.yy.c

lex.yy.c:	gplexer.lex gpparser.tab.h ast.h 
		flex gplexer.lex

main.o:         main.c pretty.h ast.h seman.h 
		$(CC) $(CFLAGS) -c main.c

ast.o: 		ast.c ast.h
		$(CC) $(CFLAGS) -c ast.c

structures.o:	structures.c ast.h structures.h
		$(CC) $(CFLAGS) -c structures.c

pretty.o:       pretty.c pretty.h ast.h seman.h
		$(CC) $(CFLAGS) -c pretty.c

seman.o:	seman.c seman.h ast.h
		$(CC) $(CFLAGS) -c seman.c

graph.o:	graph.c graph.h ast.h
		$(CC) $(CFLAGS) -c graph.c

match.o:	match.c match.h graph.h structures.h ast.h
		$(CC) $(CFLAGS) -c match.c

testgraph.o:	testgraph.c graph.h ast.h
		$(CC) $(CFLAGS) -c testgraph.c
clean:
		rm *.o gpparser.tab.c gpparser.tab.h lex.yy.c runGP

