#include "../LockedInstance.hpp"
#include <fstream>

#include <boost/python.hpp>

class PyLock {
public:
  PyLock() { myInstance = nullptr; }
  PyLock(const PyLock &aCopy) { myInstance = aCopy.myInstance; }
  PyLock &operator=(const PyLock &aCopy) = delete;
  ~PyLock() { delete myInstance; }

  double freq = 2.4e9;
  double lo_offset = 0.0;
  double rxgain = 20.0;
  double rxrate = 1e6;
  double txgain = 10.0;
  double txrate = 1e6;
  std::string addr0 = "";
  std::string addr1 = "";
  std::string addr2 = "";
  std::string addr3 = "";
  std::string rxfile = "recv";
  std::string txfile = "sin.iq";
  std::string txant = "TX/RX";
  std::string rxant = "TX/RX";
  std::string clock = "internal";

  void makeInstance() {
    uhd::device_addr_t uhdAddr;
    if ("" == addr0) {
      std::cout << "no addr specified, going with default" << '\n';
      uhdAddr = uhd::device_addr_t(addr0);
    } else {
      if ("" != addr0) {
        uhdAddr["addr0"] = addr0;
      }
      if ("" != addr1) {
        uhdAddr["addr1"] = addr1;
      }
      if ("" != addr2) {
        uhdAddr["addr2"] = addr2;
      }
      if ("" != addr3) {
        uhdAddr["addr3"] = addr3;
      }
    }
    auto source = locker::clockSources::internal;
    if ("internal" == clock) {
      source = locker::clockSources::internal;
    } else if ("external" == clock) {
      source = locker::clockSources::external;
    } else if ("mimo" == clock) {
      source = locker::clockSources::mimo;
    }
    myInstance =
        new locker::LockedInstance(freq, lo_offset, rxgain, txgain, rxrate,
                                   txrate, uhdAddr, source, rxant, txant);
    std::cout << "Instance constructed successfully." << '\n';
  }

  void clearQueue() { commandQueue.clear(); }

  void queueRx(int samples) {
    commandQueue.push_back(new locker::Receiver(samples));
    rxChannels = 1;
    std::cout << "RX of " << samples << " samples queued." << '\n';
  }

  void queueMultiRx(int samples, boost::python::list pyChannels) {
    auto channels = std::vector<size_t>(
        boost::python::stl_input_iterator<size_t>(pyChannels),
        boost::python::stl_input_iterator<size_t>());
    commandQueue.push_back(new locker::Receiver(samples, channels));
    rxChannels = channels.size();
    std::cout << "RX of " << samples << " samples across " << channels.size()
              << " channels queued." << '\n';
  }

  void queueTx(int samples) {
    auto txbuffer = std::make_shared<std::vector<std::complex<float>>>(samples);
    std::ifstream infile;
    infile.open(txfile, std::ofstream::binary);
    infile.read((char *)&txbuffer->front(),
                samples * sizeof(std::complex<float>));
    commandQueue.push_back(new locker::Transmitter(txbuffer, samples));
    std::cout << "TX of " << samples << " samples from " << txfile << " queued."
              << '\n';
  }

  void queueTxChan(int samples, int channel) {
    auto txbuffer = std::make_shared<std::vector<std::complex<float>>>(samples);
    std::ifstream infile;
    infile.open(txfile, std::ofstream::binary);
    infile.read((char *)&txbuffer->front(),
                samples * sizeof(std::complex<float>));
    commandQueue.push_back(new locker::Transmitter(txbuffer, samples, channel));
    std::cout << "TX on channel " << channel << " of " << samples
              << " samples from " << txfile << " queued." << '\n';
  }

  void queueSet(std::string setting = "rxgain", int value = 30) {
    locker::Setter *setter;
    locker::SettingType theType;
    if ("rxgain" == setting) {
      theType = locker::SettingType::rxgain;
    } else if ("txgain" == setting) {
      theType = locker::SettingType::txgain;
    } else if ("rxrate" == setting) {
      theType = locker::SettingType::rxrate;
    } else if ("txrate" == setting) {
      theType = locker::SettingType::txrate;
    } else if ("rxfreq" == setting) {
      theType = locker::SettingType::rxfreq;
    } else if ("txfreq" == setting) {
      theType = locker::SettingType::txfreq;
    } else {
      std::cout << "unknown setting '" << setting << "', no command queued"
                << std::endl;
      return;
    }
    setter = new locker::Setter(theType, value);
    commandQueue.push_back(setter);
    std::cout << "Setting queued." << '\n';
  }

  void execute(float time = 0.1, float interval = 0.0) {
    myInstance->sendTimed(commandQueue, time, interval);
    if (rxChannels) {
      writeRxResult();
    }
    commandQueue.clear();
    rxChannels = 0;
  }

  void execute_list(boost::python::list triggerTimes) {
    auto times = std::vector<double>(
        boost::python::stl_input_iterator<float>(triggerTimes),
        boost::python::stl_input_iterator<float>());
    myInstance->sendTimed(commandQueue, times);
    if (rxChannels) {
      writeRxResult();
    }
    commandQueue.clear();
    rxChannels = 0;
  }

protected:
  std::vector<locker::ITimeable *> commandQueue;
  locker::LockedInstance *myInstance;
  size_t rxChannels = 0;

  void writeRxResult() {
    for (locker::ITimeable *command : commandQueue) {
      if (command->type == locker::TimeableType::RX) {
        auto rx = dynamic_cast<locker::Receiver *>(command);
        std::vector<std::ofstream> outputs;
        for (size_t i = 0; i < rx->channels.size(); i++) {
          outputs.push_back(std::ofstream(rxfile + std::to_string(i) + ".iq",
                                          std::ofstream::binary));
          outputs.back().write((const char *)&rx->buffer[i].front(),
                               rx->samples * sizeof(std::complex<float>));
        }
      }
    }
  }
};

using namespace boost::python;

BOOST_PYTHON_MODULE(lockpy) {
  class_<PyLock>("PyLock", init<>())
      .def("make_instance", &PyLock::makeInstance)
      .def("clear_queue", &PyLock::clearQueue)
      .def("queue_rx", &PyLock::queueRx)
      .def("queue_multi_rx", &PyLock::queueMultiRx)
      .def("queue_tx", &PyLock::queueTx)
      .def("queue_tx_chan", &PyLock::queueTxChan)
      .def("queue_set", &PyLock::queueSet)
      .def("execute", &PyLock::execute)
      .def("execute_list", &PyLock::execute_list)
      .def_readwrite("freq", &PyLock::freq)
      .def_readwrite("rxgain", &PyLock::rxgain)
      .def_readwrite("rxrate", &PyLock::rxrate)
      .def_readwrite("txgain", &PyLock::txgain)
      .def_readwrite("txrate", &PyLock::txrate)
      .def_readwrite("addr0", &PyLock::addr0)
      .def_readwrite("addr1", &PyLock::addr1)
      .def_readwrite("addr2", &PyLock::addr2)
      .def_readwrite("addr3", &PyLock::addr3)
      .def_readwrite("rxant", &PyLock::rxant)
      .def_readwrite("txant", &PyLock::txant)
      .def_readwrite("clock", &PyLock::clock)
      .def_readwrite("txfile", &PyLock::txfile)
      .def_readwrite("rxfile", &PyLock::rxfile);
}
