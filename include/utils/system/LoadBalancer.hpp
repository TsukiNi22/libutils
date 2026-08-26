/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 26/08/2026 by @author Tsukini

File Name:
##  @file LoadBalancer.hpp

File Description:
##  LoadBalancer and sub class definition
\**************************************************************/

#ifndef LOADBALANCER_H
    #define LOADBALANCER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"        // utils::security::observer::Observer
    #include "../attribute/Attribute.hpp"               // _cold, _hot, _nodiscard, _unlikely, _likely, _unused
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Type, utils::exception::InternalCode
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "Scheduler.hpp"                            // utils::system::Scheduler
    #include "IdHandler.hpp"                            // utils::system::IdHandler
    #include <unordered_map>                            // std::unordered_map
    #include <type_traits>                              // std::is_base_of_v
    #include <optional>                                 // std::optional
    #include <cstddef>                                  // std::size_t
    #include <thread>                                   // std::jthread
    #include <future>                                   // std::future, std::promise
    #include <atomic>                                   // std::atomic
    #include <chrono>                                   // std::chrono::time_point, std::chrono::steady_clock::now
    #include <vector>                                   // std::vector
    #include <mutex>                                    // std::mutex, std::lock_guard

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* SUB-CLASS */

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

//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class LoadBalancer: private utils::security::observer::Observer<"LoadBalancer">  {
    static_assert(std::is_base_of_v<utils::system::Worker, T>, "T must derive from utils::system::Worker");
    private:
        /* destruction */
        utils::system::Scheduler _scheduler;
        std::jthread _thread; // Thread start at the construction, every lifespan time it's check the workers elasped time and schedule destruction
        utils::system::IdHandler<std::size_t> _idHandler;

        /* workers */
        std::size_t _limit = 1; // limit of workers (0 = infinite)
        std::chrono::milliseconds _lifespan{0}; // elasped time (in ms) before killing unused workers (0 = infinite)
        std::mutex _lock; // worker lock
        std::unordered_map<std::size_t, std::pair<T, std::size_t>> _workers;
        std::vector<std::jthread> _searchThreads;

        // ------------ Function ---------- //
        _cold void setup(void) // Setup internal thread
        {
            this->_thread = std::jthread([this](std::stop_token stoken) {
                while (!stoken.stop_requested()) _likely {
                    // Only loop every cycle of lifespan to schedule/cancel death
                    std::this_thread::sleep_for(this->_lifespan);

                    // Check for each free worker
                    std::lock_guard lock(this->_lock);
                    for (auto& [id, pair]: this->_workers) {
                        auto& [worker, task] = pair;

                        // If the worker is not working, schedule it's death
                        if (!worker.isWorking()) {
                            // Ignore thoese already scheduled
                            if (task != 0) _likely {continue;}

                            // Schedule it to death
                            std::chrono::steady_clock::time_point timestamp = worker.getStopedWorkingTimestamp();
                            std::chrono::milliseconds elasped = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timestamp);
                            std::chrono::milliseconds delay = this->_lifespan - elasped;
                            task = this->_scheduler.schedule(delay, [this, id](void){
                                std::lock_guard lock(this->_lock);
                                this->_workers.erase(id);
                                this->_idHandler.free(id);
                            });
                        }
                    }
                }
            });
        };
        std::optional<T&> findWorker(std::stop_token stoken) // async
        {
            // While nothing has been found
            while (!stoken.stop_requested()) {
                std::this_thread::yield(); // to not take all the cpu computing

                // Check each worker
                std::lock_guard lock(this->_lock);
                for (auto& [_, pair]: this->_workers) {
                    auto& [worker, task] = pair;
                    if (worker.isWorking()) _likely {continue;} // ignore thoese working

                    // Cancel the scheduled death adn change it's status
                    this->_scheduler.cancel(task);
                    worker.setStatus(true);

                    return worker;
                }
            }

            // No worker found: interrupted
            return std::nullopt;
        };

    public:
        // ------------ Function ---------- //
        std::future<T&> getWorker(void) // return an worker that can do the work (async)
        {
            std::shared_ptr<std::promise<T&>> promise = std::make_shared<std::promise<T&>>();
            std::future<T&> future = promise->get_future();

            // Purge any finished thread
            std::erase_if(this->_searchThreads, [](std::jthread& t) {return (!t.joinable() || t.get_stop_token().stop_requested());});

            // Async free worker getting
            this->_searchThreads.emplace_back([this, promise](std::stop_token stoken) {
                std::optional<T&> worker = this->findWorker(stoken); // bloque jusqu'à dispo
                if (worker.has_value()) promise->set_value(*worker);
                else promise->set_exception(std::make_exception_ptr(utils::exception::ErrorException(utils::exception::InternalCode::PromiseCanceled)));
            }); // Store them to be able to stop the research if the main class is destroy

            return future;
        };

        /* controls */
        _cold inline void kill(void) // kill all workers
        {
            std::lock_guard lock(this->_lock);
            this->_idHandler.free();
            this->_workers.clear();
        };
        _cold void kill(std::size_t n = 1) // kill n workers
        {
            std::lock_guard lock(this->_lock);
            if (this->_workers.size() < n) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds);
            }
            auto it = this->_workers.begin();
            for (std::size_t i = 0; i < n; ++i) {
                this->_idHandler.free(it->first);
                it = this->_workers.erase(it);
            }
        }
        _cold void spawn(std::size_t n = 1) // spawn n new workers
        {
            std::lock_guard lock(this->_lock);
            if (this->_workers.size() + n > this->_limit) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds);
            }
            this->_workers.reserve(this->_workers.size() + n);
            for (std::size_t i = 0; i < n; ++i) this->_workers.emplace(this->_idHandler.allocate(), std::pair<T, std::size_t>{});
        }

        /* getter/setter */
        _cold inline void setLimit(std::size_t limit) {this->_limit = limit;};
        _cold inline void setLifespan(std::chrono::milliseconds lifespan) {this->_lifespan = lifespan;};
        _cold _nodiscard inline std::size_t getLimit(void) const {return this->_limit;};
        _cold _nodiscard inline std::chrono::milliseconds getLifespan(void) const {return this->_lifespan;};
 
        // ------------ Operator ---------- //
        LoadBalancer& operator=(const LoadBalancer& other) = delete;
        LoadBalancer& operator=(LoadBalancer&& other) = default;

        // ---------- Constructor --------- //
        LoadBalancer() {this->setup();};
        LoadBalancer(std::size_t limit = 1, std::chrono::milliseconds lifespan = std::chrono::milliseconds{0}): _limit{limit}, _lifespan{lifespan} {this->setup();};
        LoadBalancer(const LoadBalancer& other) = delete;
        LoadBalancer(LoadBalancer&& other) = default;

        // ----------- Destructor --------- //
        ~LoadBalancer() = default;
};

} // namespace end
#endif /* LOADBALANCER_H */
