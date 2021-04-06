/**
 * Created by Raini Wu on April 5th, 2021.
 * Contact me at: ryw003@ucsd.edu
 */

#include "../LockedInstance.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <boost/math/special_functions/round.hpp>
#include "wavetable.hpp"

namespace examples {
  /**
   * Simultaneous TX/RX. Should output a continuous complex sinusoid.
   */
  void loopback(const std::string& outname="loopback.iq", const double& freq = 2.4e9, const double& rxgain=20.0, const double& txgain=10.0) {
    locker::LockedInstance anInstance(freq, 0.0, rxgain, txgain, 2e6, 2e6);
    std::cout << "constructed successfully" << std::endl;

    // initialize buffer
    size_t samples = 2e6;
    std::vector<std::complex<float>> buffer(samples);
    std::vector<std::complex<float>> txbuff(samples);

    // fill txbuff with sinusoid
    const wave_table_class sinusoid("SINE", 0.3);
    const size_t step = boost::math::iround(500 / 2e6 * wave_table_len);
    size_t index = 0;
    for (size_t n = 0; n < txbuff.size(); n++) {  
      txbuff[n] = sinusoid(index += step);     
    }
    
    // set up output file
    std::ofstream outfile;
    outfile.open(outname, std::ofstream::binary);

    // initialize Timeable commands
    auto receiver = std::make_shared<locker::Receiver>(buffer, samples);
    auto transmit = std::make_shared<locker::Transmitter>(txbuff, samples);

    std::vector<locker::ITimeable*> commands; 
    commands.push_back(receiver.get());
    commands.push_back(transmit.get());

    // queue commands
    anInstance.sendTimed(commands, 0.1, 0.0);
    
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
