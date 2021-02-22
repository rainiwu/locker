#include "LockedInstance.hpp"
#include <iostream>

int main() {
  locker::LockedInstance anInstance(24e6);
  std::cout << "constructed successfully" << std::endl;
  size_t samples = 500;
  std::vector<std::complex<float>> buffer(samples);
  auto receiver = std::make_shared<locker::Receiver>(buffer, samples);
  std::vector<locker::ITimeable*> commands; 
  commands.push_back(receiver.get());
  anInstance.sendTimed(commands, 2.0);
  std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1000*5.0)));
  if(buffer.empty()) {
    std::cout << "Buffer is empty!" << '\n';
  } else {
    std::cout << "Received " << buffer.size() << " packets" << std::endl;
  }
}
