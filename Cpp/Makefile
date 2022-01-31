CC := gcc
CPP := g++
CFLAGS := -llldb
OBJS := ./bin/object
SAMPLE_TARGET := ./sample/sample.c
SAMPLE_OBJS := ./sample/bin/sample
run:
	$(CPP) main.cpp lldbapi.cpp -std=gnu++17 -lpthread -g $(CFLAGS) -o $(OBJS)
	$(OBJS)
samp:
	$(CC) $(SAMPLE_TARGET) -g -o $(SAMPLE_OBJS)
clean:
	$(RM) ./bin/test
	