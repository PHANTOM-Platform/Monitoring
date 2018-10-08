ARCH   := $(shell getconf LONG_BIT)

ifeq (${ARCH},$(filter ${ARCH},32 64))
   NVML_LIB := /usr/lib64/
else
   $(error Unknown architecture!)
endif

CC = gcc

COPT_SO = $(CFLAGS) -fPIC

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(CORE_INC) $(CURL_INC) $(PUBLISH_INC) $(NVML_INC)

LFLAGS =  -lm -lpthread $(PUBLISH) $(CURL) -lnvidia-ml  -L $(NVML_LIB)

COMMON = ${CURDIR}/..

PUBLISH_INC = -I${COMMON}/publisher/src
CORE_INC = -I$(COMMON)/core
CURL_INC = -I$(COMMON)/../bin/curl/include 
NVML_INC = -I${COMMON}/../nvidia_gdk_download/usr/include/nvidia/gdk -lm 

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

libmf.so: ${SRC}/mf_api.o ${SRC}/resources_monitor.o ${SRC}/disk_monitor.o ${SRC}/power_monitor.o ${SRC}/nvml_monitor.o 
	$(CC) -shared -o $@ $^ $(CFLAGS) $(LFLAGS)

libmf.a: ${SRC}/mf_api.o ${SRC}/resources_monitor.o ${SRC}/disk_monitor.o ${SRC}/power_monitor.o ${SRC}/nvml_monitor.o  
	ar rcs $@ $^

clean:
	rm -rf ${SRC}/*.o
	rm -rf *.o *.a *.so
