CC = gcc

COPT_SO = $(CFLAGS) -fPIC

CFLAGS = -std=gnu99 -pedantic -Wall -Wwrite-strings -Wpointer-arith \
-Wcast-align -O0 -ggdb $(APR_INC) $(CORE_INC)

LFLAGS =  -lm $(APR)

COMMON = ${CURDIR}/..
CORE_INC = -I$(COMMON)/core
BINARIES = ${CURDIR}/../../bin
APR_CONFIG = $(BINARIES)/apr/bin/apr-1-config
APU_CONFIG = $(BINARIES)/apr/bin/apu-1-config
APR = $(shell $(APR_CONFIG) --link-ld) $(shell $(APU_CONFIG) --link-ld)
APR_INC = $(shell $(APR_CONFIG) --includes) $(shell $(APR_CONFIG) --includes)

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g
else
	CFLAGS += -DNDEBUG
endif

all: clean libparser.so libparser.a

mf_parser.o:
	$(CC) -c src/mf_parser.c libs/ini/ini.c $(COPT_SO) $(LFLAGS)

ini.o:
	$(CC) -c libs/ini/ini.c $(COPT_SO) $(LFLAGS)

libparser.so: mf_parser.o ini.o
	$(CC) -shared -o $@ $^ -lrt -ldl -Wl,-rpath,$(BINARIES)/apr/lib $(CFLAGS) $(LFLAGS)

libparser.a: mf_parser.o ini.o
	ar rcs $@ $^

clean:
	rm -rf *.o *.a *.so
