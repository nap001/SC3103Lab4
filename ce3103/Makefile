# Top-level Makefile for CE3103 / SC3103 Lab 4

SUBDIRS = Practice_A Practice_B Practice_C Exercise Challenge Shared_Buffer

all:
	@for dir in $(SUBDIRS); do \
		echo "=== Building $$dir ==="; \
		$(MAKE) -C $$dir; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		echo "=== Cleaning $$dir ==="; \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: all clean $(SUBDIRS)
