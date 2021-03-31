CXX=g++
CXXFLAGS=-g -Wall -pedantic -fPIC
BIN=locked
LDLIBS=-L/home/raini/doc/test/uhd-x400/host/build/lib -l:libuhd.so.4.0.0 -lpthread

SRC=Timeable.cpp LockedInstance.cpp main.cpp
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -o $(BIN) $^ $(LDLIBS)

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)
