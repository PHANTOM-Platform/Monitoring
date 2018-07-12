#Makefile for compiling example_pthreads.cpp
#If you find any missing library when compiling means that the setup-client.sh miss to install some of the required packages
#After fix any missing package  compile first the phantom_monitoring_client, then if finish successfully run this makefile again.
MF_Client_Path := /home/jmontana/phantom_mf/Monitoring_client

CC:=gcc
OPTIONS:=-O0 -g -Wall
LDLIBS := -pthread  
SRC_DIR:=src
DST_DIR:=bin

srcfiles := $(shell find ./src/ -maxdepth 3 -name "*.cpp")
objects  := $(patsubst %.c, %.o, $(srcfiles))

COMMON = $(MF_Client_Path)/src
MF_API = -L$(COMMON)/api -lmf
PUBLISH = -L${COMMON}/publisher -lpublisher
CURL = -L$(COMMON)/../bin/curl/lib -lcurl
appname=pthread-example.c

all: $(appname)

default: $(appname)

$(appname): $(objects)
	mkdir -p $(DST_DIR)
	cp $(MF_Client_Path)/src/api/src/mf_api.h src/
	$(CC) $(OPTIONS) $(LDLIBS) $(SRC_DIR)/$(appname) $(SRC_DIR)/monitoring_support.c -o $(DST_DIR)/$(basename $(appname)) $(objects) $(MF_API) $(PUBLISH) $(CURL)
	
clean:
	rm $(DST_DIR)/$(basename $(appname))

dist-clean: clean
	rm -f *~ .depend

