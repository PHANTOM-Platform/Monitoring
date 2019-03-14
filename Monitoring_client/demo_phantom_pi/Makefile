## Copyright 2014-2018 University of Stuttgart
## by J.M. Montañana

MF_Client_Path := $(PWD)

ARCH   := $(shell getconf LONG_BIT)

SRC = ${CURDIR}/src
BIN = ${CURDIR}/bin

COMMON = $(MF_Client_Path)/../src
MF_API = -L$(COMMON)/api -lmf
PARSER = -L${COMMON}/parser -lparser
PUBLISH = -L${COMMON}/publisher -lpublisher
MFUTIL = -L${COMMON}/util -lmfutil
CURL = -L$(COMMON)/../bin/curl/lib -lcurl

PARSER_INC = -I${COMMON}/parser/src
PUBLISH_INC = -I${COMMON}/publisher/src
MFUTIL_INC = -I${COMMON}/util/src
CORE_INC = -I$(COMMON)/core
CURL_INC = -I$(COMMON)/../bin/curl/include

CXX := gcc
CXXFLAGS := -pedantic -Wall -Wwrite-strings -Wpointer-arith -Wcast-align -O0 -Wall -g $(CORE_INC) $(CURL_INC) $(PUBLISH_INC) $(PARSER_INC) $(MFUTIL_INC)
LDLIBS := $(MF_API) $(PUBLISH) $(CURL) $(PARSER) $(MFUTIL) -std=gnu99 -O2 -march=native -fomit-frame-pointer -lm -lgmp -lpthread 

# srcfiles := $(shell find ./ -maxdepth 1 -name "*.c")
# objects  := $(patsubst %.c, %.o, $(srcfiles))

all: pi_simple

#pthread-example

${SRC}/%.o: ${SRC}/%.c
	$(CC) -c $< -o $@ $(LDLIBS)

pi_simple:  $(SRC)/pi_code.o 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(BIN)/$@ $(SRC)/pi_code.o $(SRC)/$@.c $(LDLIBS)

pthread-example: 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(BIN)/$@ $(SRC)/$@.c $(LDLIBS)

depend: $(srcfiles)
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	rm -f ${SRC}/*.o ${BIN}/pi_simple ${BIN}/pthread-example .depend

#include .depend
