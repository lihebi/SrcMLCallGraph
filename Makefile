CC=g++
CFLAGS = -g -Wall -std=c++11
LIBS = -lpugi -lboost_system -lboost_filesystem
# SRCS = majority_element.cpp
SRCS = $(wildcard *.cc)
# SRCS = $(filter-out support.c, $(wildcard *.c))
OBJS = $(SRCS:.cc=.o)
MAIN = cg

.PHONY: all clean


all: $(MAIN)
	@echo Compiled $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

test: test.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o test test.cpp

%.o : %.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(MAIN) test
