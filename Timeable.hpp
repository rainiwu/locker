/**
 * Created by Raini Wu, February 22nd, 2021
 * Contact me at: ryw003@ucsd.edu
 */

#ifndef Timeable_hpp
#define Timeable_hpp

#include<uhd/usrp/multi_usrp.hpp>

namespace locker {

  enum class TimeableType { RX=0, TX=1, none=99 };

  /** Timeable commands interface 
   * A Timeable command has an overloaded functor which sends some commands
   * to a given USRP which execute at a given time.
   */
  struct ITimeable {
    ITimeable(TimeableType type=TimeableType::none);
    virtual void operator()(uhd::usrp::multi_usrp::sptr& aUSRP,
        const uhd::time_spec_t& sendTime)=0;
    TimeableType type;
  };

  /** Timeable recieve to buffer */
  struct Receiver : public ITimeable {
    Receiver(std::vector<std::complex<float>>& buffer, size_t samples=0);
    ~Receiver();
    virtual void operator()(uhd::usrp::multi_usrp::sptr& aUSRP,
        const uhd::time_spec_t& sendTime);
    std::vector<std::complex<float>>& buffer;
    size_t samples;
    uhd::rx_metadata_t metadata;
  protected:
    void readToBuf();
    uhd::rx_streamer::sptr rxStreamer;
    uhd::time_spec_t myTime;
  };

  /** Timeable transmit from buffer */
  struct Transmitter : public ITimeable {
    Transmitter(const std::vector<std::complex<float>>& buffer, size_t samples=0);
    virtual void operator()(uhd::usrp::multi_usrp::sptr& aUSRP,
        const uhd::time_spec_t& sendTime);
    const std::vector<std::complex<float>>& buffer;
    size_t samples;
    uhd::tx_metadata_t metadata;
  };

}

#endif
