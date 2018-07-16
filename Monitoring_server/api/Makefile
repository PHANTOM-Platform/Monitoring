CC = gcc

COPT_SO = $(CFLAGS) -fPIC

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(CORE_INC) $(CURL_INC) $(PUBLISH_INC) 

LFLAGS =  -lm -lpthread $(PUBLISH) $(CURL)

COMMON = ${CURDIR}/..

PUBLISH_INC = -I${COMMON}/publisher/src
CORE_INC = -I$(COMMON)/core
CURL_INC = -I$(COMMON)/../bin/curl/include

PUBLISH = -L${COMMON}/publisher -lpublisher
CURL = -L$(COMMON)/../bin/curl/lib -lcurl

SRC = ${CURDIR}/src

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

all: clean libmf.so libmf.a

${SRC}/%.o: ${SRC}/%.c
	$(CC) -c $< -o $@ $(COPT_SO)

libmf.so: ${SRC}/mf_api.o ${SRC}/resources_monitor.o ${SRC}/disk_monitor.o ${SRC}/power_monitor.o 
	$(CC) -shared -o $@ $^ $(CFLAGS) $(LFLAGS)

libmf.a: ${SRC}/mf_api.o ${SRC}/resources_monitor.o ${SRC}/disk_monitor.o ${SRC}/power_monitor.o 
	ar rcs $@ $^

clean:
	rm -rf ${SRC}/*.o
	rm -rf *.o *.a *.so
