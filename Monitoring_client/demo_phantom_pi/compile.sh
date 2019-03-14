#!/bin/bash

make clean;

cp ../src/util/src/mf_util.h src;
cp ../src/api/src/mf_api.h src;
cp ../src/publisher/src/publisher.h src;
cp ../src/parser/src/mf_parser.h src;
cp ../src/core/mf_debug.h src;

#The compilation process:
make VERBOSE=1 --makefile=./Makefile
