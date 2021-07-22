/**
 * Created by Raini Wu, February 22nd, 2021
 * Contact me at: ryw003@ucsd.edu
 */

#include "Timeable.hpp"
#include <thread>

namespace locker {

  ITimeable::ITimeable(TimeableType type) : type(type) {}

  //---------------------------------------------------------------------

  Setter::Setter(SettingType setting, double value) : ITimeable(TimeableType::settings), mySetting(setting), value(value) {} 
  
  Setter::~Setter(){}

  void Setter::operator()(uhd::usrp::multi_usrp::sptr& aUSRP, 
      const uhd::time_spec_t& sendTime) {
    aUSRP->set_command_time(sendTime);
    
    // timed commands in general are not blocking, so thread not needed
    switch(mySetting) {
      case SettingType::rxgain:
        for(size_t chan=0; chan < aUSRP->get_rx_num_channels(); chan++) {
          aUSRP->set_rx_gain(value, chan);
        }
        break;
      case SettingType::txgain:
        for(size_t chan=0; chan < aUSRP->get_tx_num_channels(); chan++) {
          aUSRP->set_tx_gain(value, chan);
        }
        break;
      case SettingType::rxfreq:
        for(size_t chan=0; chan < aUSRP->get_rx_num_channels(); chan++) {
          aUSRP->set_rx_freq(uhd::tune_request_t(value, 0), chan);
        }
        break;
      case SettingType::txfreq:
        for(size_t chan=0; chan < aUSRP->get_tx_num_channels(); chan++) {
          aUSRP->set_tx_freq(uhd::tune_request_t(value, 0), chan);
        }
        break;
      case SettingType::rxrate:
        for(size_t chan=0; chan < aUSRP->get_rx_num_channels(); chan++) {
          aUSRP->set_rx_rate(value, chan);
        }
        break;
      case SettingType::txrate:
        for(size_t chan=0; chan < aUSRP->get_tx_num_channels(); chan++) {
          aUSRP->set_tx_rate(value, chan);
        }
        break;
      default:
        break;
    }
    aUSRP->clear_command_time();
    std::cout << "Set command queued. \n";
  }

  //---------------------------------------------------------------------

  Receiver::Receiver(size_t samples, std::vector<size_t> channels) : 
      ITimeable(TimeableType::RX), samples(samples), channels(channels) {
    genBuffer();
  }

  void Receiver::genBuffer() {
    buffer = std::vector<std::vector<std::complex<float>>>(
        channels.size(), std::vector<std::complex<float>>(samples));
    for(size_t i = 0; i < buffer.size(); i++) {
      bufferPtrs.push_back(&buffer[i].front());
    }
  }

  void Receiver::operator()(uhd::usrp::multi_usrp::sptr& aUSRP,
      const uhd::time_spec_t& sendTime) {
    active = true; // set ITimeable active flag
    if(nullptr == rxStreamer) {
      uhd::stream_args_t args("fc32", "sc16"); // set receive to 32bit complex float
      args.channels = channels;
      rxStreamer = aUSRP->get_rx_stream(args); 
    } else { std::cout << "note: using same rxStreamer as first Receiver" << '\n'; }
    
    // set stream type based on number of samples requested
    uhd::stream_cmd_t streamCmd((samples==0) 
        ? uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS
        : uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    streamCmd.num_samps = samples;
    streamCmd.stream_now = false;
    streamCmd.time_spec = sendTime;
    rxStreamer->issue_stream_cmd(streamCmd);

    std::cout << "RX command queued." << '\n';
    myTime = sendTime;

    // reading data into buffer blocks the thread, so spin it off
    std::thread readIn(&Receiver::readToBuf, this);
    readIn.detach();
  }

  Receiver::~Receiver() {}

  void Receiver::readToBuf() {
    if(nullptr == rxStreamer) { throw "rxStreamer not initialized"; }
    while(true == reading) {
      std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1)));
    }
    reading = true;
    rxStreamer->recv(bufferPtrs, samples, metadata, 10.0);
    if(metadata.error_code != metadata.ERROR_CODE_NONE) { std::cout << metadata.strerror() << " error occurred" << std::endl; }
    std::cout << "Difference between queued time and first packet: " << (metadata.time_spec - myTime).get_real_secs() * 1e6 << "us" << '\n';
    reading = false;
    active = false;
  }
  
  //---------------------------------------------------------------------

  Transmitter::Transmitter(const std::vector<std::complex<float>>& buffer, size_t samples, size_t channel) :
    ITimeable(TimeableType::TX), buffer(buffer), samples(samples), channel(channel) {}

  Transmitter::~Transmitter(){}

  void Transmitter::operator()(uhd::usrp::multi_usrp::sptr& aUSRP,
      const uhd::time_spec_t& sendTime) {
    active = true; // set ITimeable active flag
    uhd::stream_args_t args("fc32", "sc16"); 
    args.channels = {channel};
    txStreamer = aUSRP->get_tx_stream(args);

    metadata.start_of_burst = false;
    metadata.end_of_burst = true;
    metadata.has_time_spec = true;
    metadata.time_spec = sendTime;

    std::cout << "TX command queued." << '\n';
    myTime = sendTime;

    // send is blocking, so spin it out
    std::thread sendOut(&Transmitter::sendFromBuf, this);
    sendOut.detach();
  }

  void Transmitter::sendFromBuf() {
    size_t samplesSent = 0; // track total number of samples sent 
    while(samplesSent < samples) {
      size_t samplesToSend = std::min(samples - samplesSent, buffer.size());
      samplesSent += txStreamer->send(&buffer.front(), samplesToSend, metadata, 10.0);
      metadata.has_time_spec = false; // send subsequent packets immediately
    }
    metadata.end_of_burst = true; // send EOB
    txStreamer->send("", 0, metadata);
    std::cout << "TX data transmitted." << '\n';
    active = false;
  }
}
