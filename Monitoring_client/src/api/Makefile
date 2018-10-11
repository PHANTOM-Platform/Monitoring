ARCH   := $(shell getconf LONG_BIT)

CC = gcc

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(CORE_INC) $(CURL_INC) $(PUBLISH_INC)

COPT_SO = $(CFLAGS) -fPIC

SRC = ${CURDIR}/src

COMMON = ${CURDIR}/..

PUBLISH_INC = -I${COMMON}/publisher/src
CORE_INC = -I$(COMMON)/core
CURL_INC = -I$(COMMON)/../bin/curl/include 

PUBLISH = -L${COMMON}/publisher -lpublisher
CURL = -L$(COMMON)/../bin/curl/lib -lcurl

#
# NVML if not defined then assing 0
#
NVML ?= 0
ifneq ($(NVML),1)
   override NVML=0
   override filelist := ${SRC}/resources_monitor.o ${SRC}/disk_monitor.o ${SRC}/power_monitor.o ${SRC}/mf_api.o
   override LFLAGS = -lm -lpthread $(PUBLISH) $(CURL)
else
   NVML_INC = -I${COMMON}/../nvidia_gdk_download/usr/include/nvidia/gdk -lm 
   CFLAGS += $(NVML_INC)
   ifeq (${ARCH},$(filter ${ARCH},32 64))
      NVML_LIB := /usr/lib64/
   else
      $(error Unknown architecture!)
   endif
   override filelist :=${SRC}/resources_monitor.o ${SRC}/disk_monitor.o ${SRC}/power_monitor.o ${SRC}/nvml_monitor.o ${SRC}/mf_api.o 
   override LFLAGS = -lm -lpthread $(PUBLISH) $(CURL) -lnvidia-ml  -L $(NVML_LIB)
endif

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

all: clean libmf.so libmf.a

${SRC}/%.o: ${SRC}/%.c
	$(info $$NVML is [${NVML}])
	$(info xxxx is [$(CC) -c $< -o $@ $(COPT_SO)])
	$(CC) -c $< -o $@ $(COPT_SO)

libmf.so: ${filelist} 
	$(CC) -shared -o $@ $^ $(CFLAGS) $(LFLAGS)

libmf.a: ${filelist}
	ar rcs $@ $^

clean:
	rm -rf ${SRC}/*.o
	rm -rf *.o *.a *.so
