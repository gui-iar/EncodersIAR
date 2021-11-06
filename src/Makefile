# Compiler
CC = gcc

# Includes
INC = ../include

# Flags:
CFLAGS = -I$(INC)
CFLAGS += -Wall

SRCS=$(wildcard *.c)

OBJS=$(SRCS:.c=.o)

all: enco_to_net

enco_to_net: $(SRCS)
	gcc -o ../bin/$@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o
	rm -f ../bin/enco_to_net
	rmdir ../bin