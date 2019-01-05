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

all: clean prepare mf_Linux_resources_client mf_plugin_Linux_resources.so libmf_plugin_Linux_resources.a

clean:
	rm -rf *.o *.so
	rm -f mf_Linux_resources_client
	rm -rf ${LIB}

prepare: 
	@mkdir -p ${LIB}
	
mf_Linux_resources_client: ${SRC}/utils/mf_Linux_resources_client.c ${SRC}/mf_Linux_resources_connector.c
	${CC} $^ -o $@ ${CFLAGS} ${LFLAGS}

mf_Linux_resources_connector.o: ${SRC}/mf_Linux_resources_connector.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_Linux_resources.o: ${SRC}/mf_plugin_Linux_resources.c
	${CC} -c $< -o $@ ${COPT_SO}

mf_plugin_Linux_resources.so: mf_Linux_resources_connector.o mf_plugin_Linux_resources.o
	${CC} -shared $^ -o ${LIB}/mf_plugin_Linux_resources.so ${LFLAGS}

libmf_plugin_Linux_resources.a: mf_Linux_resources_connector.o mf_plugin_Linux_resources.o
	ar rcs ${LIB}/$@ $^
