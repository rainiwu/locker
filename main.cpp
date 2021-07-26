#include "examples/loopback.cpp"
#include "examples/multi2rx.cpp"
#include "examples/multiloop.cpp"
#include "examples/timedset.cpp"
#include "examples/tworx.cpp"
#include <string>

int main(int argc, const char *argv[]) {
  if (argc > 1) {
    std::string command(argv[1]);
    if ("loopback" == command or "lp" == command) {
      std::cout << "Running loopback test:" << '\n';
      examples::loopback();
    } else if ("timedset" == command) {
      std::cout << "Running timedset test:" << '\n';
      examples::timedset();
    } else if ("tworx" == command) {
      std::cout << "Running tworx test:" << '\n';
      examples::tworx();
    } else if ("multiloop" == command or "mlp" == command) {
      std::cout << "Running multiloop test:" << '\n';
      // initialize device addr
      uhd::device_addr_t addr;
      addr["addr0"] = "192.168.12.2";
      addr["addr1"] = "192.168.11.2";
      addr["addr2"] = "192.168.10.2";

      examples::multiloop(addr);
    } else if ("multi2rx" == command or "m2r" == command) {
      std::cout << "Running multi2rx test:" << '\n';
      // initialize device addr
      uhd::device_addr_t addr;
      addr["addr0"] = "192.168.12.2";
      addr["addr1"] = "192.168.11.2";
      addr["addr2"] = "192.168.10.2";

      examples::multi2rx(addr);
    } else {
      std::cout << "invalid argument " << command << '\n';
    }
  } else {
    std::cout << "valid arguments include: loopback, multiloop, timedset, "
                 "tworx, multi2rx"
              << std::endl;
  }
  return 0;
}
