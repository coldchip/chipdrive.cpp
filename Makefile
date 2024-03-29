CC      := g++
LD      := ld
BIN     := bin
SRCS    := $(wildcard *.cpp)
EXE     := $(BIN)/chipdrive
CFLAGS  := -Wall -std=c++11 -Ofast -s
LIBS    := -lpthread
ifeq ($(OS),Windows_NT)
	LIBS := $(LIBS) -lws2_32
endif

.PHONY: clean install

all: $(EXE)

$(EXE): $(SRCS) | $(BIN)
	cd htdocs/bin && tar -cvf ../../$(BIN)/tmp_rootdocs.tar *
	$(LD) -r -b binary -o $(BIN)/tmp_rootdocs.o bin/tmp_rootdocs.tar
	$(CC) $(CFLAGS) $(SRCS) $(BIN)/tmp_rootdocs.o $(LIBS) -o $@
	rm -rf $(BIN)/tmp_*
run:
	$(EXE)

install:
	-systemctl stop chipdrive
	-mkdir -p /etc/chipdrive
	cp install/chipdrive.service /etc/systemd/system
	cp bin/chipdrive /usr/local/bin
	systemctl daemon-reload
clean:
	rm -rf bin/*

upload:
	scp -r /home/ryan/chiphttp/* ryan@192.168.0.100:/home/ryan/chipdrive