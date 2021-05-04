#include "../LockedInstance.hpp"
#include <fstream>

#include <boost/python.hpp>

class PyLock {
public:
  PyLock(){ myInstance = nullptr; }
  PyLock(const PyLock& aCopy){ myInstance = aCopy.myInstance; }
  PyLock& operator=(const PyLock& aCopy) = delete;
  ~PyLock(){
    delete myInstance;
  }

  double freq = 2.4e9;
  double rxgain = 20.0;
  double rxrate = 1e6;
  double txgain = 10.0;
  double txrate = 1e6;
  std::string rxfile = "recv.iq";

  void makeInstance() {
    myInstance = new locker::LockedInstance(freq, 0.0, rxgain, txgain, rxrate, txrate);
    std::cout << "Instance constructed successfully." << '\n';
  }

  void clearQueue() { commandQueue.clear(); }

  void queueRx(int samples) {
    rxbuffer.clear();
    rxbuffer.resize(samples);
    commandQueue.push_back(new locker::Receiver(rxbuffer, samples));
    std::cout << "RX of " << samples << " samples queued." << '\n';
  }

  void queueTx(std::string filename, size_t samples=2e6) {
    txbuffer.clear();
    txbuffer.resize(samples);
    std::ifstream infile;
    infile.open(filename, std::ofstream::binary);
    infile.read((char*)&txbuffer.front(), samples * sizeof(std::complex<float>));
    commandQueue.push_back(new locker::Transmitter(txbuffer, samples));
    std::cout << "TX of " << samples << " samples queued." << '\n'; 
  }

  void queueSet(std::string setting="rxgain", int value=30) {
    locker::Setter *setter;
    if("rxgain" == setting) { 
      setter = new locker::Setter(locker::SettingType::rxgain, value);
      commandQueue.push_back(setter);
    }
    std::cout << "Setting queued." << '\n';
  }

  void execute(float time=0.1, float interval=0.0) {
    myInstance->sendTimed(commandQueue, time, interval);
    std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(1000*5.0 + time + interval*commandQueue.size())));
    if(!rxbuffer.empty()) {
      std::ofstream outfile;
      outfile.open(rxfile, std::ofstream::binary);
      outfile.write((const char*)&rxbuffer.front(), rxbuffer.size()*sizeof(std::complex<float>));
      outfile.close();
    }
  }

protected:
  std::vector<std::complex<float>> rxbuffer;
  std::vector<std::complex<float>> txbuffer;
  std::vector<locker::ITimeable*> commandQueue;
  locker::LockedInstance* myInstance;
};


using namespace boost::python;

BOOST_PYTHON_MODULE(lockpy) {
  class_<PyLock>("PyLock", init<>())
    .def("make_instance", &PyLock::makeInstance)
    .def("clear_queue", &PyLock::clearQueue)
    .def("queue_rx", &PyLock::queueRx)
    .def("queue_set", &PyLock::queueSet)
    .def("execute", &PyLock::execute)
    .def_readwrite("freq", &PyLock::freq)
    .def_readwrite("rxgain", &PyLock::rxgain)
    .def_readwrite("rxrate", &PyLock::rxrate)
    .def_readwrite("txgain", &PyLock::txgain)
    .def_readwrite("txrate", &PyLock::txrate)
    .def_readwrite("rxfile", &PyLock::rxfile);
}
