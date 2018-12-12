## Copyright 2014-2018 University of Stuttgart and J.M. Montanana
# author J.M. Montañana

CC = gcc
CXX = g++

#
## includes and libs
#
COMMON = ${CURDIR}/src

COPT_SO = $(CFLAGS) -fPIC

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb

LFLAGS = -lm

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

all: clean libmfutil.so libmfutil.a

mf_util.o:
	$(CC) -c src/mf_util.c $(COPT_SO) $(LFLAGS)

libmfutil.so: mf_util.o
	$(CC) -shared -o $@ $^ -lrt -ldl $(CFLAGS) $(LFLAGS)

libmfutil.a: mf_util.o
	ar rcs $@ $^

clean:
	rm -rf *.o *.a *.so
