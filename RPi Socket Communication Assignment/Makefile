TARGET = prog
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
FILENAME = rtes_rpisc_p2p

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $(FILENAME)

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(FILENAME)

clean:
	-rm -f *.o
	-rm -f $(TARGET)