#include "LockedInstance.hpp"
#include <iostream>
#include <algorithm>

int main() {
  locker::LockedInstance anInstance(24e6);
  std::cout << "constructed successfully" << std::endl;

  // initialize buffer
  size_t samples = 500;
  std::vector<std::complex<float>> buffer(samples);
  std::vector<std::complex<float>> txbuff(samples);

  // fill txbuff with random data
  std::srand(unsigned(std::time(nullptr)));
  std::generate(txbuff.begin(), txbuff.end(), std::rand);

  // initialize Timeable commands
  auto receiver = std::make_shared<locker::Receiver>(buffer, samples);
  auto transmit = std::make_shared<locker::Transmitter>(txbuff, samples);

  std::vector<locker::ITimeable*> commands; 
  commands.push_back(receiver.get());
  commands.push_back(transmit.get());

  // queue command
  anInstance.sendTimed(commands, 2.0, 2.0);
  
  // wait for execution
  std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1000*5.0)));

  // check RX result
  if(buffer.empty()) {
    std::cout << "Buffer is empty!" << '\n';
  } else {
    std::cout << "Received " << buffer.size() << " samples" << '\n';
    std::cout << "RX sample entry: " << buffer[300] << std::endl;
  }
}
