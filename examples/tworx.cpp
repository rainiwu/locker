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
   * Two RX events, writes both to one file. RX1 receives
   * a sinusoid from 6e5 to 9e5 samples into collection.
   * RX2 receives a sinsoid from 9e5 to 12e5 samples.
   * Expected loss of 3e5 samples in between RX1 and RX2.
   */
  void tworx(const std::string& outname="tworx.iq", const double& freq = 2.4e9, const double& rxgain=20.0, const double& txgain=10.0) {
    locker::LockedInstance anInstance(freq, 0.0, rxgain, txgain, 2e6, 2e6);
    std::cout << "constructed successfully" << std::endl;

    // initialize buffer
    size_t samples = 9e5;
    std::vector<std::complex<float>> rx1buf(samples);
    std::vector<std::complex<float>> rx2buf(samples);
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
    auto rx1 = std::make_shared<locker::Receiver>(rx1buf, samples);
    auto rx2 = std::make_shared<locker::Receiver>(rx2buf, samples);
    auto transmit = std::make_shared<locker::Transmitter>(txbuff, samples);

    std::vector<locker::ITimeable*> commands; 
    commands.push_back(rx1.get());
    commands.push_back(transmit.get());
    commands.push_back(rx2.get());

    // queue commands
    anInstance.sendTimed(commands, 0.1, 0.3);
    
    // wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1000*5.0)));

    std::cout << "writing to file" << std::endl;
    outfile.write((const char*)&rx1buf.front(), samples * sizeof(std::complex<float>));
    outfile.write((const char*)&rx2buf.front(), samples * sizeof(std::complex<float>));
  }
}
