########################################################################
# These lines should control what goes in the library, and the
# name of the library.
######################################################################## 
LIBNAME = polyglob.a
OBJS = polyglob.o chardecoder.o 


COMPILER = g++
LINKER = g++ -o
LINKEROPTIONS = -cvq 
LIBRARIAN = ar
INCLUDES =  -I .  
CPPOPTS =  -c -g -O2 -Wfatal-errors -Wswitch-default -Wswitch-enum -Wunused-parameter -Wfloat-equal -Wundef -Wstrict-null-sentinel -std=c++0x -pedantic -Wall -Wextra

LIBVERSION = 1
#
#########################################################################
# The following should be the same for every library.
#########################################################################
#
# LINKEROPTIONS = -shared -Wl,-soname,$(LIBNAME).$(LIBVERSION)
#
$(LIBNAME): $(OBJS)
	touch $(LIBNAME).$(LIBVERSION)
	rm $(LIBNAME).$(LIBVERSION)
	$(LIBRARIAN) $(LINKEROPTIONS) $(LIBNAME).$(LIBVERSION) $(OBJS)
	ln -s $(LIBNAME).$(LIBVERSION) $(LIBNAME) 

$(OBJS): %.o: %.cpp
	$(COMPILER) $(CPPOPTS) $(INCLUDES) $< -o $@

clean:
	rm -f *.o
	make
