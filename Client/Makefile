## Copyright 2014-2017 University of Stuttgart

CC = gcc
CXX = g++

#
# compile and link flags 
#
COPT_SO = ${CFLAGS} -fpic

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb -pthread -D_LARGEFILE64_SOURCE \
$(CURL_INC) \
$(PAPI_INC) \
$(APR_INC) \
$(PARSER_INC) \
$(PUBLISHER_INC) \
$(CORE_INC) \
$(EXCESS_QUEUE) \
$(EXCESS_QUEUE_C)

LFLAGS =  -lm $(CURL) $(PAPI) $(APR) $(PARSER) $(PUBLISHER)

#
# DEBUG SWITCH
#
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

#
# includes and libs
#
PWD = ${CURDIR}

CURL = -L$(PWD)/bin/curl/lib/ -lcurl
CURL_INC = -I$(PWD)/bin/curl/include

PAPI = -L$(PWD)/bin/papi/lib/ -lpapi
PAPI_INC = -I$(PWD)/bin/papi/include

APR_CONFIG = $(PWD)/bin/apr/bin/apr-1-config
APU_CONFIG = $(PWD)/bin/apr/bin/apu-1-config
APR = $(shell $(APR_CONFIG) --link-ld) $(shell $(APU_CONFIG) --link-ld)
APR_INC = $(shell $(APR_CONFIG) --includes) $(shell $(APR_CONFIG) --includes)

PARSER = -L$(PWD)/src/parser/ -lparser
PARSER_INC = -I$(PWD)/src/parser/src

PUBLISHER = -L$(PWD)/src/publisher/ -lpublisher
PUBLISHER_INC = -I$(PWD)/src/publisher/src

CORE_INC = -I$(PWD)/src/core

EXCESS_QUEUE = -I$(PWD)/ext/queue/data-structures-library/src/include
EXCESS_QUEUE_C = -I$(PWD)/ext/queue

#
# main source files
#
SRC = $(PWD)/src/agent
FILES = $(shell find $(SRC) -name "*.c")
HEADER = $(shell find $(SRC) -name "*.h")
PLUGIN_DIR = $(PWD)/src/plugins

#
# build and install paths
#
INSTALL_DIR = $(PWD)/dist
INSTALL_BIN_DIR = $(INSTALL_DIR)/bin
INSTALL_PLUGINS_DIR = $(INSTALL_DIR)/bin/plugins
INSTALL_LIB_DIR = $(INSTALL_DIR)/lib

#
# compile and build
#
all: prepare mf_api main plugins lib

${SRC}/%.o: %.c ${HEADER}
	$(CC) -c $< $(COPT_SO)

excess_concurrent_queue.o:
	$(CXX) -c $(PWD)/ext/queue/excess_concurrent_queue.cpp -o $@ -I. $(EXCESS_QUEUE) $(EXCESS_QUEUE_C) -fpic

prepare:
	$(MAKE) -C $(PWD)/src/parser DEBUG=$(DEBUG)
	$(MAKE) -C $(PWD)/src/publisher DEBUG=$(DEBUG)
mf_api:
	$(MAKE) -C $(PWD)/src/api DEBUG=$(DEBUG)
	$(MAKE) -C $(PWD)/src/api/test DEBUG=$(DEBUG)

main: excess_concurrent_queue.o $(SRC)/main.o $(SRC)/thread_handler.o $(SRC)/plugin_discover.o $(SRC)/plugin_manager.o
	$(CXX) -o $@ $^ -lrt -ldl -Wl,--export-dynamic $(CFLAGS) $(LFLAGS)

plugins:
	$(MAKE) -C $(PLUGIN_DIR)/Board_power DEBUG=$(DEBUG)
	$(MAKE) -C $(PLUGIN_DIR)/CPU_perf DEBUG=$(DEBUG)
	$(MAKE) -C $(PLUGIN_DIR)/CPU_temperature DEBUG=$(DEBUG)
	$(MAKE) -C $(PLUGIN_DIR)/Linux_resources DEBUG=$(DEBUG)
	$(MAKE) -C $(PLUGIN_DIR)/Linux_sys_power DEBUG=$(DEBUG)
	$(MAKE) -C $(PLUGIN_DIR)/NVML DEBUG=$(DEBUG)

lib:

#
# clean-up
#
clean-all: clean clean-install

clean:
	rm -rf *.o *.a *.so $(SRC)/*.o
	rm -rf $(PWD)/ext/queue/*.o
	rm -f $(PWD)/main
	$(MAKE) -C $(PWD)/src/parser clean
	$(MAKE) -C $(PWD)/src/publisher clean
	$(MAKE) -C $(PWD)/src/api clean
	$(MAKE) -C $(PWD)/src/api/test clean
	$(MAKE) -C $(PLUGIN_DIR)/Board_power clean
	$(MAKE) -C $(PLUGIN_DIR)/CPU_perf clean
	$(MAKE) -C $(PLUGIN_DIR)/CPU_temperature clean
	$(MAKE) -C $(PLUGIN_DIR)/Linux_resources clean
	$(MAKE) -C $(PLUGIN_DIR)/Linux_sys_power clean
	$(MAKE) -C $(PLUGIN_DIR)/NVML clean

clean-install:
	rm -rf $(INSTALL_DIR)

#
# install
#
install: clean-install prepare-install copy_ini copy_plugins copy_client copy_libs

prepare-install:
	@mkdir -p $(INSTALL_BIN_DIR)
	@mkdir -p $(INSTALL_PLUGINS_DIR)
	@mkdir -p $(INSTALL_LIB_DIR)

copy_ini:
	cp -f $(PWD)/src/mf_config.ini $(INSTALL_DIR)

copy_plugins:
	cp -f $(PLUGIN_DIR)/Board_power/lib/*.so $(INSTALL_PLUGINS_DIR)/
	cp -f $(PLUGIN_DIR)/CPU_perf/lib/*.so $(INSTALL_PLUGINS_DIR)/
	cp -f $(PLUGIN_DIR)/CPU_temperature/lib/*.so $(INSTALL_PLUGINS_DIR)/
	cp -f $(PLUGIN_DIR)/Linux_resources/lib/*.so $(INSTALL_PLUGINS_DIR)/
	cp -f $(PLUGIN_DIR)/Linux_sys_power/lib/*.so $(INSTALL_PLUGINS_DIR)/
	cp -f $(PLUGIN_DIR)/NVML/lib/*.so $(INSTALL_PLUGINS_DIR)/ 2>/dev/null || :

copy_client:
	cp -f $(PWD)/main $(INSTALL_BIN_DIR)/mf_client

copy_libs:
	cp -f $(PWD)/bin/apr/lib/lib*.so* $(INSTALL_LIB_DIR)
	cp -f $(PWD)/bin/curl/lib/lib*.so* $(INSTALL_LIB_DIR)
	cp -f $(PWD)/bin/hwloc/lib/lib*.so* $(INSTALL_LIB_DIR)
	cp -f $(PWD)/bin/libiio/lib/lib*.so* $(INSTALL_LIB_DIR)
	cp -f $(PWD)/bin/papi/lib/lib*.so* $(INSTALL_LIB_DIR)
	cp -f $(PWD)/bin/sensors/lib/lib*.so* $(INSTALL_LIB_DIR)
	cp -f $(PWD)/src/parser/lib*.so* $(INSTALL_LIB_DIR)
	cp -f $(PWD)/src/publisher/lib*.so* $(INSTALL_LIB_DIR)
	cp -f $(PWD)/src/api/lib*.so* $(INSTALL_LIB_DIR)
