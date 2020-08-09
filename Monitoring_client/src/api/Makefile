## Copyright 2014-2018 University of Stuttgart
# Updated by J.M. Montañana

CC = gcc
CXX = g++

#
## includes and libs
#
ARCH	:= $(shell getconf LONG_BIT)

SRC = ${CURDIR}/src

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

#
# compile and link flags
#
CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
	-Wcast-align -O0 -ggdb \
	$(CORE_INC) $(CURL_INC) $(PUBLISHER_INC) $(PARSER_INC) $(MFUTIL_INC)

LDLIBS := $(PUBLISHER) $(CURL) $(PARSER) $(MFUTIL)

COPT_SO = ${CFLAGS} -fpic

LFLAGS =  -lm $(PAPI) $(APR) $(LDLIBS)

#
# NVML if not defined then assing 0
#
NVML ?= 0
ifneq ($(NVML),1)
	override NVML=0
	override filelist := ${SRC}/linux_resources.o ${SRC}/xlnx_monitor.o ${SRC}/linux_sys_power.o ${SRC}/mf_api.o
	override LFLAGS = -lm -lpthread $(LDLIBS)
else
	NVML_INC = -I${COMMON}/../nvidia_gdk_download/usr/include/nvidia/gdk -lm
	CFLAGS += $(NVML_INC)
	ifeq (${ARCH},$(filter ${ARCH},32 64))
		NVML_LIB := /usr/lib64/
	else
		$(error Unknown architecture!)
	endif
	override filelist :=${SRC}/linux_resources.o ${SRC}/xlnx_monitor.o ${SRC}/linux_sys_power.o ${SRC}/nvml_monitor.o ${SRC}/mf_api.o
	override LFLAGS = -lm -lpthread $(LDLIBS) -lnvidia-ml -L $(NVML_LIB)
endif

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
all: clean libmf.so libmf.a

${SRC}/%.o: ${SRC}/%.c
	$(info $$NVML is [${NVML}])
	$(info xxxx is [$(CC) -c $< -o $@ $(COPT_SO)])
	$(CC)  -c $< -o $@ $(COPT_SO)

libmf.so: ${filelist}
	$(CC) -shared -o $@ $^ $(CFLAGS) $(LFLAGS)

libmf.a: ${filelist}
	ar rcs $@ $^

clean:
	rm -rf ${SRC}/*.o
	rm -rf *.o *.a *.so
