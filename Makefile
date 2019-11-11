# constants
CC = g++
CFLAGS = -I. -Wall
OUT = sponge

sponge: sponge.cc sponge.h
	$(CC) -o $(OUT) $(CFLAGS) sponge.cc

install: sponge configure
	ln -s $(shell pwd)/sponge /home/${USER}/.local/bin/sponge 

clean:
	rm sponge
