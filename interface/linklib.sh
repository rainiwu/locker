cd ..
g++ -Wall -pedantic -I /usr/include/python3.8 -fPIC -c -o PyLock.o ./interface/PyLock.cpp
g++ -shared -o lockpy.so LockedInstance.o PyLock.o Timeable.o main.o -luhd -lpthread -lpython3.8 -lboost_python38
