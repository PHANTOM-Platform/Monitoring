##
## Copyright (C) 2018 University of Stuttgart
##
CC = gcc
#
# includes and libs
#
SRC = ${CURDIR}/src
LIB = ${CURDIR}/lib

PARSER_INC = -I${CURDIR}/../../parser/src
UTILS_INC = -I${CURDIR}/../utils
AGENT_INC = -I${CURDIR}/../../agent
EXCESS_QUEUE_INC = -I${CURDIR}/../../../ext/queue
CORE_INC = -I${CURDIR}/../../core 

#
# compile and link flags
#
CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings  -Wpointer-arith -Wcast-align -O0 -ggdb \
-I${SRC} ${PARSER_INC} ${UTILS_INC} ${AGENT_INC} ${EXCESS_QUEUE_INC} ${CORE_INC}

COPT_SO = ${CFLAGS} -fpic

LFLAGS = -lrt -ldl

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

all: clean prepare mf_dummy_client mf_plugin_dummy.so libmf_plugin_dummy.a

clean:
	rm -rf *.o *.so
	rm -f mf_dummy_client
	rm -rf ${LIB}

prepare: 
	@mkdir -p ${LIB}

mf_dummy_client: ${SRC}/utils/mf_dummy_client.c ${SRC}/mf_dummy_connector.c
	${CC} $^ -o $@ ${CFLAGS} ${LFLAGS}
	
mf_dummy_connector.o: ${SRC}/mf_dummy_connector.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_dummy.o: ${SRC}/mf_plugin_dummy.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_dummy.so: mf_dummy_connector.o mf_plugin_dummy.o
	${CC} -shared $^ -o ${LIB}/mf_plugin_dummy.so ${LFLAGS}

libmf_plugin_dummy.a: mf_dummy_connector.o mf_plugin_dummy.o
	ar rcs ${LIB}/$@ $^
