# Tools (e.g.: bison, flex, gcc, etc.)
YACC = bison
LEX  = flex 
CC   = gcc

# Directories
INCLDIR     = ../include
LIBDIR      = ../lib

# Flags for tools
YACC_OPTIONS = -d -v
CFLAGS      = -O3 -I $(INCLDIR) -L $(LIBDIR) -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast

# Files
OBJECTS = commandLines.o printSourceCode.o handleBisonRules.o offlineScheduler.o auxiliary.o SGFunctions.o NSGA_Functions.o

# Clean command
RM = rm -r -f


switches:	switches.y scanner.fl $(OBJECTS)
			$(YACC) $(YACC_OPTIONS) switches.y
			$(LEX) scanner.fl
			$(CC) $(CFLAGS) -o switches lex.yy.c switches.tab.c $(OBJECTS) -lfl

commandLines.o:	commandLines.c 
				$(CC) $(CFLAGS) -c commandLines.c

printSourceCode.o:	printSourceCode.c 
				$(CC) $(CFLAGS) -c printSourceCode.c
				
handleBisonRules.o:	handleBisonRules.c 
				$(CC) $(CFLAGS) -c handleBisonRules.c

SGFunctions.o:	SGFunctions.c 
				$(CC) $(CFLAGS) -c SGFunctions.c				
				
NSGA_Functions.o:	NSGA_Functions.c 
				$(CC) $(CFLAGS) -c NSGA_Functions.c

auxiliary.o:	auxiliary.c 
				$(CC) $(CFLAGS) -c auxiliary.c

offlineScheduler.o:	offlineScheduler.c 
				$(CC) $(CFLAGS) -c offlineScheduler.c


clean:
	$(RM) *.o
	$(RM) *.a
	$(RM) switches
	$(RM) *.exe
	$(RM) lex.yy.c
	$(RM) switches.tab.c
	$(RM) switches.tab.h
	$(RM) switches.output
