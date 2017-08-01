# Application Name
APP		= sw_rk4

# Directories
INCLUDE     	= sw.h

# Tools
CC 		= gcc

# Flags for tools
CFLAGS      	= -O3
LIBS		= -lpthread

# Files
OBJECTS = sw_threads.o sw_threadpool.o

# Clean command
RM = rm -r -f


$(APP):	$(OBJECTS) $(INCLUDE)
		$(CC) $(CFLAGS) $(APP).c -o $(APP) $(OBJECTS) $(LIBS)

sw_threads.o:	sw_threads.c $(INCLUDE)
			$(CC) $(CFLAGS) -c sw_threads.c $(LIBS)

sw_threadpool.o:	sw_threadpool.c $(INCLUDE)
			$(CC) $(CFLAGS) -c sw_threadpool.c $(LIBS)
			
			

clean:
	$(RM) sw_*
	$(RM) sw.h
	$(RM) *.log
	$(RM) *.o
	$(RM) *.a
	$(RM) *.exe
	$(RM) *.gch
	$(RM) *~
	$(RM) $(APP)
