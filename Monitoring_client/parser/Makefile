## Copyright 2014-2018 University of Stuttgart
# Updated by J.M. Montañana

CC = gcc
CXX = g++

#
## includes and libs
#
COMMON = ${CURDIR}/..

PARSER_INC = -I${COMMON}/parser/src
PUBLISHER_INC = -I${COMMON}/publisher/src
MFUTIL_INC = -I${COMMON}/util/src
CORE_INC = -I$(COMMON)/core
CURL_INC = -I$(COMMON)/../bin/curl/include

PARSER = -L${COMMON}/parser -lparser
PUBLISHER = -L${COMMON}/publisher -lpublisher
MFUTIL = -L${COMMON}/util -lmfutil
CURL = -L$(COMMON)/../bin/curl/lib -lcurl

BINARIES = ${CURDIR}/../../bin
APR_CONFIG = $(BINARIES)/apr/bin/apr-1-config
APU_CONFIG = $(BINARIES)/apr/bin/apu-1-config
APR = $(shell $(APR_CONFIG) --link-ld) $(shell $(APU_CONFIG) --link-ld)
APR_INC = $(shell $(APR_CONFIG) --includes) $(shell $(APR_CONFIG) --includes)
#
# compile and link flags
#
CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(APR_INC) $(CORE_INC) $(MFUTIL_INC)

COPT_SO = $(CFLAGS) -fPIC

LFLAGS = -lm $(APR) $(MFUTIL)

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
all: clean libparser.so libparser.a

mf_parser.o:
	$(CC) -c src/mf_parser.c libs/ini/ini.c $(COPT_SO) $(LFLAGS)

ini.o:
	$(CC) -c libs/ini/ini.c $(COPT_SO) $(LFLAGS)

libparser.so: mf_parser.o ini.o
	$(CC) -shared -o $@ $^ -lrt -ldl -Wl,-rpath,$(BINARIES)/apr/lib $(CFLAGS) $(LFLAGS)

libparser.a: mf_parser.o ini.o
	ar rcs $@ $^

clean:
	rm -rf *.o *.a *.so
