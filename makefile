CXX=g++
CXXFLAGS=-Wall -pedantic
BIN=locked
LDLIBS=-luhd -lpthread

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -o $(BIN) $^ $(LDLIBS)

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)
