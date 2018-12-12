## Copyright 2014-2018 University of Stuttgart
# Updated by J.M. Montañana

CC = gcc
CXX = g++

#
## includes and libs
#
COMMON = ${CURDIR}/../..

PARSER_INC = -I${COMMON}/parser/src
PUBLISHER_INC = -I${COMMON}/publisher/src
MFUTIL_INC = -I${COMMON}/util/src
CORE_INC = -I$(COMMON)/core
CURL_INC = -I$(COMMON)/../bin/curl/include

PARSER = -L${COMMON}/parser -lparser
PUBLISHER = -L${COMMON}/publisher -lpublisher
MFUTIL = -L${COMMON}/util -lmfutil
CURL = -L$(COMMON)/../bin/curl/lib -lcurl

MF_API_INC = -I$(COMMON)/api/src
MF_API = -L$(COMMON)/api -lmf

#
# compile and link flags
#
CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(CORE_INC) $(CURL_INC) $(PUBLISHER_INC) $(MF_API_INC) $(PARSER_INC) $(MFUTIL_INC)

LFLAGS = -lnvidia-ml -lm -lpthread $(PUBLISHER) $(CURL) $(MF_API) $(PARSER) $(MFUTIL)

#
# DEBUG SWITCH
#
DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g
endif

#
# compile and build
#
all: test_mf_api

#test_mf_api: test_mf_api.c
#	$(CC) -o $@ $^ ${SRC}/*.c $(CFLAGS) $(LFLAGS)

test_mf_api: test_mf_api.c
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

clean:
	rm -rf test_mf_api
