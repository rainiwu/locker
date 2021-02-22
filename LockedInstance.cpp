/**
 * Created by Raini Wu, February 21st 2021
 * Contact me at: ryw003@ucsd.edu
 */

#include "LockedInstance.hpp"
#include <iostream>

namespace locker {
  LockedInstance::LockedInstance(
      const double& freq,
      const double& lo_offset,
      const double& rxgain,
      const double& txgain,
      const double& rxrate,
      const double& txrate,
      const uhd::device_addr_t& anAddr,
      const clockSources& aSource,
      const std::string& rxant,
      const std::string& txant,
      const double& setupTime
    ) {
    myUSRP = uhd::usrp::multi_usrp::make(anAddr); // init USRP ptr
    myUSRP->set_clock_source(getSource(aSource));
    uhd::tune_request_t tuneReq(freq, lo_offset); // interim type
    this->tuneAll(tuneReq, rxgain, txgain, rxrate, txrate, rxant, txant);
    myUSRP->set_time_next_pps(uhd::time_spec_t(0.0)); // set time
    std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1000 * setupTime)));
    checkAllLock(aSource);
  }

  LockedInstance::~LockedInstance() {}

  void LockedInstance::sendTimed(const std::vector<ITimeable*>& commands, double time, double interval) {
    uhd::time_spec_t triggerTime = myUSRP->get_time_now() + uhd::time_spec_t(time);
    for(auto command : commands) {
      (*command)(myUSRP, triggerTime); // timed commands don't block
      triggerTime += uhd::time_spec_t(interval);
    }
  }
  
  void LockedInstance::tuneAll(const uhd::tune_request_t& aRequest,
      const double& rxgain,
      const double& txgain,
      const double& rxrate,
      const double& txrate,
      const std::string& rxant,
      const std::string& txant
      ) {
    for(size_t chan = 0; chan < myUSRP->get_tx_num_channels(); chan++) {
      myUSRP->set_tx_freq(aRequest, chan);
      myUSRP->set_tx_gain(txgain, chan);
      myUSRP->set_tx_rate(txrate, chan);
      myUSRP->set_tx_antenna(txant, chan);
    }
    for(size_t chan = 0; chan < myUSRP->get_rx_num_channels(); chan++) {
      myUSRP->set_rx_freq(aRequest, chan);
      myUSRP->set_rx_gain(rxgain, chan);
      myUSRP->set_rx_rate(rxrate, chan);
      myUSRP->set_rx_antenna(rxant, chan);
    }
  }

  bool LockedInstance::checkAllLock(const clockSources& aSource) {
    std::vector<std::string> sensorNames;
    for(size_t chan = 0; chan < myUSRP->get_tx_num_channels(); chan++) {
      sensorNames = myUSRP->get_tx_sensor_names(chan);
      if(std::find(sensorNames.begin(), sensorNames.end(), "lo_locked") != sensorNames.end()) {
        uhd::sensor_value_t txloResult = myUSRP->get_tx_sensor("lo_locked", chan);
        std::cout << "TX Channel " << chan << " " << txloResult.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(txloResult.to_bool());
      }
    }
    for(size_t chan = 0; chan < myUSRP->get_rx_num_channels(); chan++) {
      sensorNames = myUSRP->get_rx_sensor_names(chan);
      if(std::find(sensorNames.begin(), sensorNames.end(), "lo_locked") != sensorNames.end()) {
        uhd::sensor_value_t rxloResult = myUSRP->get_rx_sensor("lo_locked", chan);
        std::cout << "RX Channel " << chan << " " << rxloResult.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(rxloResult.to_bool());
      }
    }
    for(size_t num = 0; num < myUSRP->get_num_mboards(); num++) {
      sensorNames = myUSRP->get_mboard_sensor_names(num);
      std::string checkstr;
      if(aSource == clockSources::internal) { break; }
      else if(aSource == clockSources::external) { checkstr="ref_locked"; }
      else if(aSource == clockSources::mimo) { checkstr="mimo_locked"; }
      if(std::find(sensorNames.begin(), sensorNames.end(), checkstr) != sensorNames.end()) {
        uhd::sensor_value_t refResult = myUSRP->get_mboard_sensor(checkstr, num);
        UHD_ASSERT_THROW(refResult.to_bool());
      }
    }
    return true; 
  }

  std::string LockedInstance::getSource(const clockSources& aSource) {
    return sources[static_cast<int>(aSource)];
  }
}
