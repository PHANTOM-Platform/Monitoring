## Copyright 2014-2018 University of Stuttgart
# Updated by J.M. Montañana

CC = gcc
CXX = g++

#
## includes and libs
#
COMMON = ${CURDIR}/..

PARSER_INC = -I$(COMMON)/parser/src
PUBLISHER_INC = -I$(COMMON)/publisher/src
MFUTIL_INC = -I$(COMMON)/util/src
CORE_INC = -I$(COMMON)/core
CURL_INC = -I$(COMMON)/../bin/curl/include

PARSER = -L$(COMMON)/parser -lparser
PUBLISHER = -L$(COMMON)/publisher -lpublisher
MFUTIL = -L$(COMMON)/util -lmfutil
CURL = -L$(COMMON)/../bin/curl/lib -lcurl

#
# compile and link flags
#
CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(CORE_INC) $(CURL_INC) $(MFUTIL_INC)

COPT_SO = $(CFLAGS) -fPIC

LFLAGS = -lm $(CURL) $(MFUTIL)

#
# DEBUG SWITCH
#
DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

#
# compile and build
#
all: clean libpublisher.so libpublisher.a

publisher.o:
	$(CC) -c src/publisher.c $(COPT_SO) $(LFLAGS)

libpublisher.so: publisher.o
	$(CC) -shared -o $@ $^ -lrt -ldl -Wl,-rpath,$(COMMON)/../bin/curl $(CFLAGS) $(LFLAGS)

libpublisher.a: publisher.o
	ar rcs $@ $^

clean:
	rm -rf *.o *.a *.so
