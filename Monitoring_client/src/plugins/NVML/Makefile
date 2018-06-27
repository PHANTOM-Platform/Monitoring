##
## Copyright (C) 2018 University of Stuttgart
##
CC = gcc
COPT_SO = ${CFLAGS} -fpic

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings  -Wpointer-arith -Wcast-align -O0 -ggdb \
-I${SRC} ${NVML_INC} ${PARSER_INC} ${UTILS_INC} ${AGENT_INC} ${EXCESS_QUEUE_INC} ${CORE_INC}

LFLAGS = -lrt -ldl ${NVML_LIB}

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

SRC = ${CURDIR}/src
LIB = ${CURDIR}/lib
NVML_PATH = ${CURDIR}/../../../bin/nvidia/usr
NVML_INC = -I${NVML_PATH}/include/nvidia/gdk
# should use the libnvidia-ml.so, which is installed with the NVIDIA Display Driver.
# by default the lib path is /usr/lib64 or /usr/lib

#sudo  mkdir /usr/lib64; sudo chmod 777 /usr/lib64/;
#cp ../../../bin/nvidia/usr/src/gdk/nvml/lib/libnvidia-ml.so /usr/lib64;
NVML_LIB = -L/usr/lib64 -lnvidia-ml
#NVML_LIB = -L${CURDIR}/../../../bin/nvidia/usr/src/gdk/nvml/lib -lnvidia-ml
#bin/nvidia/usr/src/gdk/nvml/lib/libnvidia-ml.so

PARSER_INC = -I${CURDIR}/../../parser/src
UTILS_INC = -I${CURDIR}/../utils
AGENT_INC = -I${CURDIR}/../../agent
EXCESS_QUEUE_INC = -I${CURDIR}/../../../ext/queue
CORE_INC = -I${CURDIR}/../../core 

all: clean prepare mf_NVML_client mf_plugin_NVML.so

mf_plugin_NVML.so: mf_NVML_connector.o mf_plugin_NVML.o
ifneq (,$(wildcard /usr/lib64/libnvidia-ml.so))
	${CC} -shared $^ -o ${LIB}/mf_plugin_NVML.so ${LFLAGS}
endif

mf_NVML_connector.o: ${SRC}/mf_NVML_connector.c
ifneq (,$(wildcard /usr/lib64/libnvidia-ml.so))
	${CC} -c $< -o $@ ${COPT_SO}
endif

mf_plugin_NVML.o: ${SRC}/mf_plugin_NVML.c
ifneq (,$(wildcard /usr/lib64/libnvidia-ml.so))
	${CC} -c $< -o $@ ${COPT_SO}
endif

mf_NVML_client: ${SRC}/utils/mf_NVML_client.c ${SRC}/mf_NVML_connector.c
ifneq (,$(wildcard /usr/lib64/libnvidia-ml.so))
	${CC} $^ -o $@ ${CFLAGS} ${LFLAGS}
endif

prepare: 
	@mkdir -p ${LIB}

clean:
	rm -rf *.o *.so
	rm -f mf_NVML_client
	rm -rf ${LIB}
