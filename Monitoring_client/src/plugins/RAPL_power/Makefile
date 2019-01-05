##
## Copyright (C) 2018 University of Stuttgart
##
CC = gcc

#
# includes and libs
#
SRC = ${CURDIR}/src
LIB = ${CURDIR}/lib

HWLOC_PATH = ${CURDIR}/../../../bin/hwloc
HWLOC_INC = -I${HWLOC_PATH}/include
HWLOC_LIB = -L${HWLOC_PATH}/lib -lhwloc

PAPI_PATH = ${CURDIR}/../../../bin/papi
PAPI_INC = -I${PAPI_PATH}/include
PAPI_LIB = -L${PAPI_PATH}/lib -lpapi

PARSER_INC = -I${CURDIR}/../../parser/src
UTILS_INC = -I${CURDIR}/../utils
AGENT_INC = -I${CURDIR}/../../agent
EXCESS_QUEUE_INC = -I${CURDIR}/../../../ext/queue
CORE_INC = -I${CURDIR}/../../core 

#
# compile and link flags
#
CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings  -Wpointer-arith -Wcast-align -O0 -ggdb \
-I${SRC} ${HWLOC_INC} ${PAPI_INC} ${PARSER_INC} ${UTILS_INC} ${AGENT_INC} ${EXCESS_QUEUE_INC} ${CORE_INC}

COPT_SO = ${CFLAGS} -fpic

LFLAGS = -lrt -ldl -Wl,-rpath,${HWLOC_PATH}:${PAPI_PATH} ${HWLOC_LIB} ${PAPI_LIB}

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

all: clean prepare mf_RAPL_power_client mf_plugin_RAPL_power.so libmf_plugin_RAPL_power.a

clean:
	rm -rf *.o *.so
	rm -f mf_RAPL_power_client
	rm -rf ${LIB}

prepare: 
	@mkdir -p ${LIB}

mf_RAPL_power_client: ${SRC}/utils/mf_RAPL_power_client.c ${SRC}/mf_RAPL_power_connector.c
	${CC} $^ -o $@ ${CFLAGS} ${LFLAGS}

mf_RAPL_power_connector.o: ${SRC}/mf_RAPL_power_connector.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_RAPL_power.o: ${SRC}/mf_plugin_RAPL_power.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_RAPL_power.so: mf_RAPL_power_connector.o mf_plugin_RAPL_power.o
	${CC} -shared $^ -o ${LIB}/mf_plugin_RAPL_power.so ${LFLAGS}

libmf_plugin_RAPL_power.a: mf_RAPL_power_connector.o mf_plugin_RAPL_power.o
	ar rcs ${LIB}/$@ $^
