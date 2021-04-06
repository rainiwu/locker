/**
 * Created by Raini Wu on April 5th, 2021.
 * Contact me at: ryw003@ucsd.edu
 */

#include "../LockedInstance.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>

namespace examples {
  /**
   * Sets RX gain to 30db at 0.5s or 1e6 samples into receive. 
   * Should result in visible amplitude change at that exact time. 
   */
  void timedset(const std::string& outname="timedset.iq", const double& freq = 2.4e9, const double& rxgain=0.0) {
    locker::LockedInstance anInstance(freq, 0.0, rxgain, 1.0, 2e6, 2e6);
    std::cout << "constructed successfully" << std::endl;

    // initialize buffer
    size_t samples = 2e6;
    std::vector<std::complex<float>> buffer(samples);

    // set up output file
    std::ofstream outfile;
    outfile.open(outname, std::ofstream::binary);

    // initialize Timeable commands
    auto receiver = std::make_shared<locker::Receiver>(buffer, samples);
    auto setter = std::make_shared<locker::Setter>(locker::SettingType::rxgain, 30);

    std::vector<locker::ITimeable*> commands; 
    commands.push_back(receiver.get());
    commands.push_back(setter.get());

    // queue command
    anInstance.sendTimed(commands, 0.1, 0.5);
    
    // wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1000*5.0)));

    // check RX result
    if(buffer.empty()) {
      std::cout << "Buffer is empty!" << '\n';
    } else {
      std::cout << "Received " << buffer.size() << " samples" << '\n';
      std::cout << "RX sample entry at 0.1M: " << buffer[0.1e6] << '\n';
      std::cout << "RX sample entry at 1.9M: " << buffer[1.9e6] << std::endl;
    }

    std::cout << "writing to file" << std::endl;
    outfile.write((const char*)&buffer.front(), samples * sizeof(std::complex<float>));
  }
}
