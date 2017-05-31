CC = gcc

COPT_SO = $(CFLAGS) -fPIC

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(CORE_INC) $(CURL_INC)

LFLAGS =  -lm $(CURL)

COMMON = ${CURDIR}/..
CORE_INC = -I$(COMMON)/core

CURL_INC = -I$(COMMON)/../bin/curl/include
CURL = -L$(COMMON)/../bin/curl/lib -lcurl

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

all: clean libpublisher.so libpublisher.a

publisher.o:
	$(CC) -c src/publisher.c $(COPT_SO) $(LFLAGS)

libpublisher.so: publisher.o
	$(CC) -shared -o $@ $^ -lrt -ldl -Wl,-rpath,$(COMMON)/../bin/curl $(CFLAGS) $(LFLAGS)

libpublisher.a: publisher.o
	ar rcs $@ $^

clean:
	rm -rf *.o *.a *.so
