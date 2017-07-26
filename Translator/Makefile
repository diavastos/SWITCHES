# Directories
SRCDIR      = src
INCLDIR     = include

# Clean command
RM = rm -r -f


all:	switches

switches:
	cd $(SRCDIR)/; make switches
	cp $(SRCDIR)/switches .
	


clean:
	$(RM) sw_*
	$(RM) *.o
	$(RM) *.a
	$(RM) *.h
	$(RM) *.exe
	$(RM) *.log
	$(RM) *.out
	$(RM) switches
	cd $(SRCDIR)/; make clean
