/**
 * Created by Raini Wu, February 21st 2021
 * Contact me at: ryw003@ucsd.edu
 */

#ifndef LockedInstance_hpp
#define LockedInstance_hpp

#include <uhd/usrp/multi_usrp.hpp>
#include <chrono>
#include <thread>

namespace locker {

  enum class clockSources { internal=0, external=1, mimo=2 };
  static const char* sources[]
    { "internal", "external", "mimo" };

  class LockedInstance {
  public:
    /**
     * Creates a USRP instance with given parameters
     */
    LockedInstance(const uhd::device_addr_t& anAddr=uhd::device_addr_t(""),
        const clockSources& aSource=clockSources::internal,
        const double& freq=0.0,
        const double& lo_offset=0.0,
        const double& rxgain=0.0,
        const double& txgain=1.0,
        const double& rxrate=1e6,
        const double& txrate=1e6,
        const std::string& rxant="TX/RX",
        const std::string& txant="TX/RX",
        const double& setupTime = 1.0);
    ~LockedInstance();

  protected:
    /** maps clockSources enum to string */
    inline std::string getSource(const clockSources& aSource); 
    /** sets freq, lo_lock, and others for all USRP TX and RX channels */
    void tuneAll(const uhd::tune_request_t& aRequest, 
        const double& rxgain,
        const double& txgain,
        const double& rxrate,
        const double& txrate,
        const std::string& rxant="TX/RX",
        const std::string& txant="TX/RX");
    /** checks all TX/RX channels for lo and sensor lock */ 
    bool checkAllLock(const clockSources& aSource);

    uhd::usrp::multi_usrp::sptr myUSRP; /** USRP instance */
  };
}

#endif
