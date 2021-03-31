CXX=g++
CXXFLAGS=-Wall -pedantic
BIN=locked
UHD4LIB=/home/raini/doc/test/uhd-x400/host/build/lib
LDLIBS=-L$(UHD4LIB) -l:libuhd.so.4.0.0 -lpthread -lboost_system

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -o $(BIN) $^ $(LDLIBS)

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)
