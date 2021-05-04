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
3. Run 'make' command
#### Building Python Interface
1. Do above
2. Run interface/linklib.sh

### File Structure 
examples - directory containing several example files
interface - directory containing Python interface code
