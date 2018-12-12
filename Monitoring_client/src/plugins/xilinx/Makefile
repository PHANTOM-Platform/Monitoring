##
## Copyright (C) 2018 University of Stuttgart
##
CC = gcc
COPT_SO = ${CFLAGS} -fpic

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings  -Wpointer-arith -Wcast-align -O0 -ggdb \
-I${SRC} ${PARSER_INC} ${UTILS_INC} ${AGENT_INC} ${EXCESS_QUEUE_INC} ${CORE_INC}

LFLAGS = -lrt -ldl

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

SRC = ${CURDIR}/src
LIB = ${CURDIR}/lib

PARSER_INC = -I${CURDIR}/../../parser/src
UTILS_INC = -I${CURDIR}/../utils
AGENT_INC = -I${CURDIR}/../../agent
EXCESS_QUEUE_INC = -I${CURDIR}/../../../ext/queue
CORE_INC = -I${CURDIR}/../../core 

all: clean prepare mf_xilinx_client mf_plugin_xilinx.so

mf_plugin_xilinx.so: mf_xilinx_connector.o mf_plugin_xilinx.o
	${CC} -shared $^ -o ${LIB}/mf_plugin_xilinx.so ${LFLAGS}

mf_xilinx_connector.o: ${SRC}/mf_xilinx_connector.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_xilinx.o: ${SRC}/mf_plugin_xilinx.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_xilinx_client: ${SRC}/utils/mf_xilinx_client.c ${SRC}/mf_xilinx_connector.c
	${CC} $^ -o $@ ${CFLAGS} ${LFLAGS}

prepare: 
	@mkdir -p ${LIB}

clean:
	rm -rf *.o *.so
	rm -f mf_xilinx_client
	rm -rf ${LIB}