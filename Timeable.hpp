/**
 * Created by Raini Wu, February 22nd, 2021
 * Contact me at: ryw003@ucsd.edu
 */

#ifndef Timeable_hpp
#define Timeable_hpp

#include <uhd/usrp/multi_usrp.hpp>

namespace locker {

enum class TimeableType { RX = 0, TX = 1, settings = 2, none = 99 };

enum class SettingType { rxgain = 0, rxfreq, rxrate, txgain, txfreq, txrate };

/** Timeable commands interface
 * A Timeable command has an overloaded functor which sends some commands
 * to a given USRP which execute at a given time.
 */
struct ITimeable {
  ITimeable(TimeableType type = TimeableType::none);
  virtual void operator()(uhd::usrp::multi_usrp::sptr &aUSRP,
                          const uhd::time_spec_t &sendTime) = 0;
  TimeableType type;
  bool active = false;
};

/** Timeable change USRP settings */
class Setter : public ITimeable {
public:
  Setter(SettingType setting, double value);
  ~Setter();

  virtual void operator()(uhd::usrp::multi_usrp::sptr &aUSRP,
                          const uhd::time_spec_t &sendTime);

protected:
  SettingType mySetting;
  double value;
};

/** Timeable recieve to buffer */
class Receiver : public ITimeable {
public:
  Receiver(size_t samples = 0, std::vector<size_t> channels = {0});
  ~Receiver(); /** prevents default double free */

  virtual void operator()(uhd::usrp::multi_usrp::sptr &aUSRP,
                          const uhd::time_spec_t &sendTime);

  /** holds rx'd data */
  std::vector<std::vector<std::complex<float>>> buffer;

  const size_t samples;
  const std::vector<size_t> channels;

  uhd::rx_metadata_t metadata; /** collects received metadata */
protected:
  void readToBuf(); /** read received data, helps threading */
  void genBuffer(); /** create buffers used in rx */

  /** ptrs passed to uhd recv */
  std::vector<std::complex<float> *> bufferPtrs;

  inline static uhd::rx_streamer::sptr rxStreamer =
      nullptr;                        /** required for threading */
  inline static bool reading = false; /** recv active flag */
  uhd::time_spec_t myTime;            /** saves queued time */
};

/** Timeable transmit from buffer */
class Transmitter : public ITimeable {
public:
  Transmitter(const std::vector<std::complex<float>> &buffer,
              size_t samples = 0, size_t channel = 0);
  ~Transmitter();

  virtual void operator()(uhd::usrp::multi_usrp::sptr &aUSRP,
                          const uhd::time_spec_t &sendTime);

  const std::vector<std::complex<float>> &buffer;
  uhd::tx_metadata_t metadata;

protected:
  void sendFromBuf();
  size_t samples;
  size_t channel;
  uhd::tx_streamer::sptr txStreamer;
  uhd::time_spec_t myTime;
};

} // namespace locker

#endif
