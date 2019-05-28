.PHONY: all
CC = g++
CPPFLAGS = -pthread -std=c++11
BIN = main

all: $(BIN)
$(BIN): threadpool.cpp condition.cpp

clean:
	-rm -rf $(BIN)
