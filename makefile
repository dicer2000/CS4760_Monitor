# Improved Makefile by Brett Huffman v1.1
# (c)2021 Brett Huffman
# To use, just change the next line to the name of
# the application

# App 1 - builds the monitor program
appname1 := monitor
srcfiles := $(shell find . -name "monitor*.cpp")
objects1  := $(patsubst %.cpp, %.o, $(srcfiles))

all: $(appname1)

$(appname1): $(objects1) libmonitor.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname1) $(objects1) $(LDLIBS)

# Static monitor library
libmonitor.a: libmonitor.o
	ar $(ARFLAGS) $@ $^

# App 2 - builds the producer program
appname2 := producer
srcfiles := $(shell find . -name "producer*.cpp")
objects2  := $(patsubst %.cpp, %.o, $(srcfiles))

all: $(appname2)

$(appname2): $(objects2)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname2) $(objects2) $(LDLIBS)

# App 3 - builds the consumer program
appname3 := consumer
srcfiles := $(shell find . -name "consumer*.cpp")
objects3  := $(patsubst %.cpp, %.o, $(srcfiles))

all: $(appname3)

$(appname3): $(objects3)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname3) $(objects3) $(LDLIBS)

clean:
	rm -f $(objects1)
	rm -f $(appname1)
	rm -f $(objects2)
	rm -f $(appname2)
	rm -f $(objects3)
	rm -f $(appname3)
	rm -f libmonitor.a
	rm -f libmonitor.o