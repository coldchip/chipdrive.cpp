CC      := g++
LD      := ld
BIN     := bin
SRCS    := $(wildcard *.cpp)
EXE     := $(BIN)/chipdrive
CFLAGS  := -Wall -std=c++11 -Ofast -s
LIBS    := -lpthread -lcurl
ifeq ($(OS),Windows_NT)
	LIBS := $(LIBS) -lws2_32
endif

.PHONY: clean

all: $(EXE)

$(EXE): $(SRCS) | $(BIN)
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) -o $@
run:
	$(EXE)

clean:
	rm -rf bin/*

upload:
	scp -r /home/ryan/chiphttp/* ryan@192.168.0.100:/home/ryan/chipdrive