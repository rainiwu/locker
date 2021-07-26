CXX=g++
CXXFLAGS=-Wall -pedantic -std=c++17 -fPIC
PYLIB=/usr/include/python3.8
UHD4LIB=/home/raini/doc/test/uhd-x400/host/build/lib

CSRC=Timeable.cpp LockedInstance.cpp main.cpp
COBJ=$(CSRC:%.cpp=%.o)
CBIN=locked
CLDLIBS=-L$(UHD4LIB) -l:libuhd.so.4.0.0 -lpthread -lboost_system

PYSRC=Timeable.cpp LockedInstance.cpp interface/PyLock.cpp
PYOBJ=$(PYSRC:%.cpp=%.o)
PYBIN=lockpy.so
PYLDLIBS=$(CLDLIBS) -lpython3.8 -lboost_python38

all: $(COBJ) $(PYOBJ)
	$(CXX) -o $(CBIN) $(COBJ) $(CLDLIBS)
	$(CXX) -shared -o $(PYBIN) $(PYOBJ) $(PYLDLIBS)

cpp: $(COBJ)
	$(CXX) -o $(CBIN) $^ $(CLDLIBS)

python: $(PYOBJ)
	$(CXX) -shared -o $(PYBIN) $^ $(PYLDLIBS)

interface/%.o: interface/%.cpp
	$(CXX) $(CXXFLAGS) -I $(PYLIB) -c -o $@ $<

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm -f interface/*.o
	rm $(CBIN)
	rm $(PYBIN)
