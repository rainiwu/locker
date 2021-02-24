/**
 * Created by Raini Wu, February 22nd, 2021
 * Contact me at: ryw003@ucsd.edu
 */

#include "Timeable.hpp"
#include <thread>

namespace locker {

  ITimeable::ITimeable(TimeableType type) : type(type) {}

  Receiver::Receiver(std::vector<std::complex<float>>& buffer, size_t samples) : ITimeable(TimeableType::RX), buffer(buffer), samples(samples) {}

  void Receiver::operator()(uhd::usrp::multi_usrp::sptr& aUSRP,
      const uhd::time_spec_t& sendTime) {
    uhd::stream_args_t args("fc32", "sc16"); // set receive to 32bit complex float
    rxStreamer = aUSRP->get_rx_stream(args); 
    
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
    rxStreamer->recv(&buffer.front(), samples, metadata, 10.0);
    if(metadata.error_code != metadata.ERROR_CODE_NONE) { std::cout << metadata.strerror() << " error occurred" << std::endl; }
    std::cout << "Difference between queued time and first packet: " << (metadata.time_spec - myTime).get_real_secs() * 1e6 << "us" << '\n';
  }
  
  //---------------------------------------------------------------------

  Transmitter::Transmitter(const std::vector<std::complex<float>>& buffer, size_t samples) : ITimeable(TimeableType::TX), buffer(buffer), samples(samples) {}

  Transmitter::~Transmitter(){}

  void Transmitter::operator()(uhd::usrp::multi_usrp::sptr& aUSRP,
      const uhd::time_spec_t& sendTime) {
    uhd::stream_args_t args("fc32", "sc16"); 
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
  }
}
