/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 27/08/2026 by @author Tsukini

File Name:
##  @file Worker.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef WORKER_H
    #define WORKER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"               // _nodiscard, _hot, _cold, _unlikely
    #include "../security/observer/Observer.hpp"        // utils::security::observer::Observer
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::* (Type)
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include <chrono>                                   // std::chrono::steady_clock::time_point, std::chrono::steady_clock::now
    #include <atomic>                                   // std::atomic

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Worker: private utils::security::observer::Observer<"Worker"> {
    private:
        std::atomic<bool> _workingStatus = false; // working status
        std::atomic<std::chrono::steady_clock::time_point> _stopedWorkingTimestamp = std::chrono::steady_clock::now(); // timestamp when the working status was passed at false

    public:
        // ------------ Function ---------- //
        _hot inline void setWorkingStatus(const bool status) {if (!(this->_workingStatus = status)) this->_stopedWorkingTimestamp = std::chrono::steady_clock::now();};
        _cold _nodiscard inline std::chrono::steady_clock::time_point getStopedWorkingTimestamp(void) const {return this->_stopedWorkingTimestamp;};
        _hot _nodiscard inline bool isWorking(void) const {return this->_workingStatus;};

        // ------------ Operator ---------- //
        Worker& operator=(_unused const Worker& other) {this->setWorkingStatus(other.isWorking()); return *this;};
        Worker& operator=(Worker&& other) {this->setWorkingStatus(other.isWorking()); other.setWorkingStatus(false); return *this;};

        // ---------- Constructor --------- //
        Worker() = default;
        Worker(_unused const Worker& other): _workingStatus{other.isWorking()} {};
        Worker(Worker&& other): _workingStatus{other.isWorking()} {other.setWorkingStatus(false);};

        // ----------- Destructor --------- //
        ~Worker() = default;
};

} // namespace end
#endif /* WORKER_H */
