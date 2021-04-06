#include <string>
#include "examples/loopback.cpp"
#include "examples/timedset.cpp"
#include "examples/tworx.cpp"

int main(int argc, const char* argv[]) {
  if(argc > 1) {
    std::string command(argv[1]);
    if("loopback" == command) {
      std::cout << "Running loopback test:" << '\n';
      examples::loopback();
    }
    else if("timedset" == command) {
      std::cout << "Running timedset test:" << '\n';
      examples::timedset();
    }
    else if("tworx" == command) {
      std::cout << "Running tworx test:" << '\n';
      examples::tworx();
    }
    else {
      std::cout << "invalid argument " << command << '\n';
    }
  } else { std::cout << "valid arguments include: loopback, timedset, tworx" << std::endl; }
  return 0;
}
