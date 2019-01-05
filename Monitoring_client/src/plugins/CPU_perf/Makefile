##
## Copyright (C) 2018 University of Stuttgart
##
CC = gcc

#
# includes and libs
#
SRC = ${CURDIR}/src
LIB = ${CURDIR}/lib

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
-I${SRC} ${PAPI_INC} ${PARSER_INC} ${UTILS_INC} ${AGENT_INC} ${EXCESS_QUEUE_INC} ${CORE_INC}

COPT_SO = ${CFLAGS} -fpic

LFLAGS = -lrt -ldl -Wl,-rpath,${PAPI_PATH} ${PAPI_LIB}

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

all: clean prepare mf_CPU_perf_client mf_plugin_CPU_perf.so libmf_plugin_CPU_perf.a

clean:
	rm -rf *.o *.so
	rm -f mf_CPU_perf_client
	rm -rf ${LIB}

prepare: 
	@mkdir -p ${LIB}

mf_CPU_perf_client: ${SRC}/utils/mf_CPU_perf_client.c ${SRC}/mf_CPU_perf_connector.c
	${CC} $^ -o $@ ${CFLAGS} ${LFLAGS}
	
mf_CPU_perf_connector.o: ${SRC}/mf_CPU_perf_connector.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_CPU_perf.o: ${SRC}/mf_plugin_CPU_perf.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_CPU_perf.so: mf_CPU_perf_connector.o mf_plugin_CPU_perf.o
	${CC} -shared $^ -o ${LIB}/mf_plugin_CPU_perf.so ${LFLAGS}
	
libmf_plugin_CPU_perf.a: mf_CPU_perf_connector.o mf_plugin_CPU_perf.o
	ar rcs ${LIB}/$@ $^
