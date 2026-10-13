# Root Makefile for SC3103 Lab 4

all:
	$(MAKE) -C ce3103 all

clean:
	$(MAKE) -C ce3103 clean

.PHONY: all clean
