CC = gcc

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(CORE_INC) $(CURL_INC) $(PUBLISH_INC) $(MF_API_INC)

LFLAGS = -lm -lpthread $(PUBLISH) $(CURL) $(MF_API)

COMMON = ${CURDIR}/../..

PUBLISH_INC = -I${COMMON}/publisher/src
CORE_INC = -I$(COMMON)/core
CURL_INC = -I$(COMMON)/../bin/curl/include
MF_API_INC = -I$(COMMON)/api/src

PUBLISH = -L${COMMON}/publisher -lpublisher
CURL = -L$(COMMON)/../bin/curl/lib -lcurl
MF_API = -L$(COMMON)/api -lmf

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g
endif

all: test_mf_api

#test_mf_api: test_mf_api.c
#	$(CC) -o $@ $^ ${SRC}/*.c $(CFLAGS) $(LFLAGS)

test_mf_api: test_mf_api.c
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)


clean:
	rm -rf test_mf_api