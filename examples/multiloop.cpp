/**
 * Created by Raini Wu on July 1st, 2021.
 * Contact me at: ryw003@ucsd.edu
 */

#include "../LockedInstance.hpp"
#include "wavetable.hpp"
#include <algorithm>
#include <boost/math/special_functions/round.hpp>
#include <fstream>
#include <iostream>

namespace examples {
/**
 * Simultaneous TX/RX. Should output a continuous complex sinusoid.
 */
void multiloop(const uhd::device_addr_t &addr,
               const std::vector<size_t> &channels = {2, 3, 4, 5},
               const double &freq = 2.4e9, const double &rxgain = 20.0,
               const double &txgain = 10.0) {
  // construct lockedInstance
  locker::LockedInstance anInstance(freq, 0.0, rxgain, txgain, 100e6, 100e6,
                                    addr, locker::clockSources::external);
  std::cout << "constructed successfully" << std::endl;

  // initialize buffer
  size_t samples = 2e6;

  std::vector<std::complex<float>> txbuff(samples);

  // fill txbuff with sinusoid
  const wave_table_class sinusoid("SINE", 0.3);
  const size_t step = boost::math::iround(40e6 / 100e6 * wave_table_len);
  size_t index = 0;
  for (size_t n = 0; n < txbuff.size(); n++) {
    txbuff[n] = sinusoid(index += step);
  }

  // set up output files
  std::ofstream out1, out2, out3, out4;
  out1.open("nl1.iq", std::ofstream::binary);
  out2.open("nl2.iq", std::ofstream::binary);
  out3.open("nl3.iq", std::ofstream::binary);
  out4.open("nl4.iq", std::ofstream::binary);

  // initialize Timeable commands
  auto receiver = std::make_shared<locker::Receiver>(samples, channels);
  auto transmit = std::make_shared<locker::Transmitter>(txbuff, samples);

  std::vector<locker::ITimeable *> commands;
  commands.push_back(receiver.get());
  commands.push_back(transmit.get());

  // queue commands
  anInstance.sendTimed(commands, 0.1, 0.0);

  std::cout << "writing to file" << std::endl;
  out1.write((const char *)&receiver->buffer[0].front(),
             samples * sizeof(std::complex<float>));
  out2.write((const char *)&receiver->buffer[1].front(),
             samples * sizeof(std::complex<float>));
  out3.write((const char *)&receiver->buffer[2].front(),
             samples * sizeof(std::complex<float>));
  out4.write((const char *)&receiver->buffer[3].front(),
             samples * sizeof(std::complex<float>));
}
} // namespace examples
