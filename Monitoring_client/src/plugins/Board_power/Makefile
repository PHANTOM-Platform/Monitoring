##
## Copyright (C) 2018 University of Stuttgart
##
CC = gcc
CXX = g++

#
# includes and libs
#
SRC = ${CURDIR}/src
LIB = ${CURDIR}/lib

LIBIIO_PATH = ${CURDIR}/../../../bin/libiio
LIBIIO_INC = -I${LIBIIO_PATH}/include
LIBIIO_LIB = -L${LIBIIO_PATH}/lib -liio

PARSER_INC = -I${CURDIR}/../../parser/src
UTILS_INC = -I${CURDIR}/../utils
AGENT_INC = -I${CURDIR}/../../agent
EXCESS_QUEUE_INC = -I${CURDIR}/../../../ext/queue
CORE_INC = -I${CURDIR}/../../core 

#
# compile and link flags
#
CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings  -Wpointer-arith -Wcast-align -O0 -ggdb \
-I${SRC} ${LIBIIO_INC} ${PARSER_INC} ${UTILS_INC} ${AGENT_INC} ${EXCESS_QUEUE_INC} ${CORE_INC}

COPT_SO = ${CFLAGS} -fpic

LFLAGS = -lrt -ldl -Wl,-rpath,${LIBIIO_PATH} ${LIBIIO_LIB}

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif


all: clean prepare mf_Board_power_client mf_plugin_Board_power.so

mf_plugin_Board_power.so: mf_Board_power_connector.o mf_plugin_Board_power.o
	${CC} -shared $^ -o ${LIB}/mf_plugin_Board_power.so ${LFLAGS}

mf_Board_power_connector.o: ${SRC}/mf_Board_power_connector.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_Board_power.o: ${SRC}/mf_plugin_Board_power.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_Board_power_client: ${SRC}/utils/mf_Board_power_client.c ${SRC}/mf_Board_power_connector.c
	${CC} $^ -o $@ ${CFLAGS} ${LFLAGS}

prepare: 
	@mkdir -p ${LIB}

clean:
	rm -rf *.o *.so
	rm -f mf_Board_power_client
	rm -rf ${LIB}
