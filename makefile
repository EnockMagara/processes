CC = gcc
CFLAGS = -Wall -g
OBJS = main.o child_handlers.o

all: event_processor

event_processor: $(OBJS)
	$(CC) $(CFLAGS) -o event_processor $(OBJS)

main.o: main.c child_handlers.h
	$(CC) $(CFLAGS) -c main.c

child_handlers.o: child_handlers.c child_handlers.h
	$(CC) $(CFLAGS) -c child_handlers.c

clean:
	rm -f $(OBJS) event_processor