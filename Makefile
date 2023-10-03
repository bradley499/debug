cc ?= gcc
name=libdebug
lib=$(name).so
ifeq ($(shell uname -s),Darwin)
	lib=$(name).dylib
endif

src=debug.c
obj=debug.o

all:
	$(cc) -Wall -os -fPIC -s -shared -o $(lib) -c $(src) 

clean:
	rm $(lib)