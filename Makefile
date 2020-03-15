# The name of the source files
SOURCES = rtes_rpisc_p2p.c rtes_rpisc_server.c rtes_rpisc_ioworker.c rtes_rpisc_dataworker.c rtes_rpisc_nodeslist.c rtes_rpisc_circularbuffer.c rtes_rpisc_rwlock.c rtes_rpisc_client.c

# The name of the executable
EXE = exe

# Flags for compilation (adding warnings are always good)
CFLAGS = -Wall -g -I.

# Flags for linking (none for the moment)
LDFLAGS =

# Libraries to link with (none for the moment)
LIBS = -levent -lpthread -levent_pthreads

# Libraries path
LIBPATH = -L/usr/local/lib/

# Use the GCC frontend program when linking
LD = gcc

# This creates a list of object files from the source files
OBJECTS = $(SOURCES:%.c=%.o)

# The first target, this will be the default target if none is specified
# This target tells "make" to make the "all" target
default: all

# Having an "all" target is customary, so one could write "make all"
# It depends on the executable program
all: $(EXE)

# This will link the executable from the object files
$(EXE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(LIBPATH) $(OBJECTS) -o  $(EXE) $(LIBS)

# This is a target that will compile all needed source files into object files
# We don't need to specify a command or any rules, "make" will handle it automatically
#%.o : %.c

# Target to clean up after us
clean:
	-rm -f $(EXE)      # Remove the executable file
	-rm -f $(OBJECTS)  # Remove the object files

# Finally we need to tell "make" what source and header file each object file depends on

