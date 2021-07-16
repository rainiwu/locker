/**
 * Created by Raini Wu on July 1st, 2021.
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
  void multi2rx(const uhd::device_addr_t& addr, const std::vector<size_t>& channels={2, 3, 4, 5}, 
      const double& freq = 2.4e9, const double& rxgain=20.0, const double& txgain=10.0) {
    // construct lockedInstance
    locker::LockedInstance anInstance(freq, 0.0, rxgain, txgain, 100e6, 100e6, addr, locker::clockSources::external);
    std::cout << "constructed successfully" << std::endl;

    // initialize buffer
    size_t rxSamples = 2e6; // receieve 20ms
    size_t txSamples = 10e6;// transmit 100ms

    std::vector<std::vector<std::complex<float>>> rx1buffs(
        channels.size(), std::vector<std::complex<float>>(rxSamples)
    );

    std::vector<std::complex<float>*> rx1buffer;
    for (size_t i = 0; i < rx1buffs.size(); i++) { 
      rx1buffer.push_back(&rx1buffs[i].front());
    }

    std::vector<std::vector<std::complex<float>>> rx2buffs(
        channels.size(), std::vector<std::complex<float>>(rxSamples)
    );

    std::vector<std::complex<float>*> rx2buffer;
    for (size_t i = 0; i < rx2buffs.size(); i++) { 
      rx2buffer.push_back(&rx2buffs[i].front());
    }

    std::vector<std::complex<float>> txbuff(txSamples);

    // fill txbuff with sinusoid
    const wave_table_class sinusoid("SINE", 0.3);
    const size_t step = boost::math::iround(40e6 / 100e6 * wave_table_len);
    size_t index = 0;
    for (size_t n = 0; n < txbuff.size(); n++) {  
      txbuff[n] = sinusoid(index += step);     
    }
    
    // set up output files
    std::ofstream out1, out2, out3, out4;
    out1.open("mlp1.iq", std::ofstream::binary);
    out2.open("mlp2.iq", std::ofstream::binary);
    out3.open("mlp3.iq", std::ofstream::binary);
    out4.open("mlp4.iq", std::ofstream::binary);

    // initialize Timeable commands
    auto transmit = std::make_shared<locker::Transmitter>(txbuff, txSamples);
    auto rx1 = std::make_shared<locker::Receiver>(rx1buffer, rxSamples, channels);
    auto rx2 = std::make_shared<locker::Receiver>(rx2buffer, rxSamples, channels);

    std::vector<locker::ITimeable*> commands; 
    commands.push_back(transmit.get());
    commands.push_back(rx1.get());
    commands.push_back(rx2.get());

    // queue commands
    anInstance.sendTimed(commands, {0.1, 0.1, 0.14});
    
    // wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1000*5.0)));

    std::cout << "writing to file" << std::endl;
    out1.write((const char*)&rx1buffs[0].front(), rxSamples * sizeof(std::complex<float>));
    out2.write((const char*)&rx1buffs[1].front(), rxSamples * sizeof(std::complex<float>));
    out3.write((const char*)&rx1buffs[2].front(), rxSamples * sizeof(std::complex<float>));
    out4.write((const char*)&rx1buffs[3].front(), rxSamples * sizeof(std::complex<float>));

    out1.write((const char*)&rx2buffs[0].front(), rxSamples * sizeof(std::complex<float>));
    out2.write((const char*)&rx2buffs[1].front(), rxSamples * sizeof(std::complex<float>));
    out3.write((const char*)&rx2buffs[2].front(), rxSamples * sizeof(std::complex<float>));
    out4.write((const char*)&rx2buffs[3].front(), rxSamples * sizeof(std::complex<float>));
  }
}
