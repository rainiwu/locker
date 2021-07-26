# locker
Lightweight C++ program designed to operate USRPs in predictable ways. Built on UHD 4.0. 

### Dependencies
* UHD 4.0
* Boost
* pthread 
* C++17
* Python3

### Build  Instructions
1. Install all dependencies
2. Modify UHD 4.0 directory in makefile
3. Modify Python version in makefile
4. Run `make` command

Build with `make cpp` or `make python` if you wish to compile only each respective part.

### File Structure 
examples - directory containing several example files

interface - directory containing Python interface code
