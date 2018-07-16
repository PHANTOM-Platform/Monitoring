appname := myapp

CXX := g++
CXXFLAGS := -Wall -g -std=c++11

srcfiles := $(shell find . -maxdepth 1 -name "*.cpp")
objects  := $(patsubst %.cpp, %.o, $(srcfiles))

COMMON = ${CURDIR}/../..
MF_API = -L$(COMMON)/api -lmf
PUBLISH = -L${COMMON}/publisher -lpublisher
CURL = -L$(COMMON)/../bin/curl/lib -lcurl

all: $(appname)

$(appname): $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname) $(objects) $(LDLIBS) $(MF_API) $(PUBLISH) $(CURL)

depend: .depend

.depend: $(srcfiles)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	rm -f $(objects) $(appname) .depend

dist-clean: clean
	rm -f *~ .depend

include .depend