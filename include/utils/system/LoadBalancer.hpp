/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/10/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"               // _cold, _hot, _nodiscard, _unlikely, _likely
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Type, utils::exception::InternalCode
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../type/Worker.hpp"                       // utils::type::Worker
    #include "IdHandler.hpp"                            // utils::system::IdHandler
    #include <condition_variable>                       // std::condition_variable_any
    #include <unordered_map>                            // std::unordered_map
    #include <type_traits>                              // std::is_base_of_v
    #include <exception>                                // std::current_exception
    #include <cstddef>                                  // std::size_t
    #include <memory>                                   // std::shared_ptr, std::make_shared
    #include <thread>                                   // std::jthread, std::this_thread::sleep_for
    #include <future>                                   // std::future, std::promise
    #include <atomic>                                   // std::atomic
    #include <chrono>                                   // std::chrono::milliseconds, std::chrono::steady_clock::now
    #include <string>                                   // std::to_string
    #include <list>                                     // std::list
    #include <mutex>                                    // std::mutex, std::lock_guard

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class LoadBalancer: private utils::security::observer::Observer<"LoadBalancer"> {
    static_assert(std::is_base_of_v<utils::type::Worker, T>, "T must derive from utils::type::Worker");
    private:
        /* asynchronous worker search (getWorker) */
        struct Search {
            std::jthread thread;
            std::shared_ptr<std::atomic<bool>> done;
        };

        /* workers */
        std::size_t _limit = 1; // limit of workers (0 = infinite)
        std::chrono::milliseconds _lifespan{0}; // elasped time (in ms) before killing unused workers (0 = infinite)
        mutable std::mutex _lock; // workers & settings lock
        utils::system::IdHandler<std::size_t> _idHandler;
        std::unordered_map<std::size_t, T> _workers;

        /* threads */
        std::list<Search> _searchs; // declared last: joined first
        std::jthread _thread; // every cycle, kill the workers unused since more than lifespan

        // ------------ Function ---------- //
        _cold void setup_(void) // Setup internal thread
        {
            this->_thread = std::jthread([this](std::stop_token stoken) {
                std::mutex mutex;
                std::condition_variable_any cv;

                while (!stoken.stop_requested()) _likely {
                    std::chrono::milliseconds lifespan = this->getLifespan();

                    // Wait a cycle (or the stop), 0 = infinite lifespan: only wait for the stop
                    {
                        std::unique_lock<std::mutex> cvlock(mutex);
                        (void)cv.wait_for(cvlock, stoken, (lifespan.count() > 0) ? lifespan : std::chrono::milliseconds{100}, [](void) {return false;});
                    }
                    if (stoken.stop_requested() || lifespan.count() == 0) continue;

                    // Kill the workers unused since more than lifespan
                    std::lock_guard lock(this->_lock);
                    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                    for (auto it = this->_workers.begin(); it != this->_workers.end();) {
                        const T& worker = it->second;
                        if (!worker.isWorking() && now - worker.getStopedWorkingTimestamp() >= this->_lifespan) {
                            this->_idHandler.free(it->first);
                            it = this->_workers.erase(it);
                        } else ++it;
                    }
                }
            });
        };
        _hot T* findWorker_(std::stop_token stoken) // async, nullptr if interrupted
        {
            while (!stoken.stop_requested()) {
                {
                    std::lock_guard lock(this->_lock);
                    for (auto &[_, worker]: this->_workers) {
                        if (worker.isWorking()) _likely {continue;} // ignore those working
                        worker.setWorkingStatus(true);
                        return &worker;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds{1}); // to not take all the cpu computing
            }
            return nullptr;
        };

    public:
        // ------------ Function ---------- //
        // Return a free worker (set as working), the worker must be set as not working when the work is done
        std::future<T&> getWorker(void) // (async)
        {
            std::shared_ptr<std::promise<T&>> promise = std::make_shared<std::promise<T&>>();
            std::future<T&> future = promise->get_future();
            std::shared_ptr<std::atomic<bool>> done = std::make_shared<std::atomic<bool>>(false);

            std::lock_guard lock(this->_lock);

            // Purge the finished searchs
            std::erase_if(this->_searchs, [](const Search& search) {return search.done->load();});

            // Async free worker getting (canceled at the destruction)
            this->_searchs.push_back(Search{std::jthread([this, promise, done](std::stop_token stoken) {
                T* worker = this->findWorker_(stoken);
                if (worker) _likely {
                    promise->set_value(*worker);
                } else _unlikely {
                    try {throw utils::exception::ErrorException(utils::exception::InternalCode::PromiseCanceled);}
                    catch (...) {promise->set_exception(std::current_exception());}
                }
                *done = true;
            }), done});

            return future;
        };

        /* controls */
        template<bool mode_forced = false> // Also kill the working ones (their T& become invalid)
        _cold inline void kill(void) // kill all the not working workers
        {
            std::lock_guard lock(this->_lock);
            for (auto it = this->_workers.begin(); it != this->_workers.end();) {
                if (!mode_forced && it->second.isWorking()) {++it; continue;}
                this->_idHandler.free(it->first);
                it = this->_workers.erase(it);
            }
        };
        template<bool mode_forced = false> // Also kill the working ones (their T& become invalid)
        _cold void kill(std::size_t n) // kill n workers (the not working ones first)
        {
            std::lock_guard lock(this->_lock);
            std::size_t idle = 0;
            for (const auto &[_, worker]: this->_workers) idle += !worker.isWorking();
            if ((mode_forced ? this->_workers.size() : idle) < n) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Not enough workers: " + std::to_string(mode_forced ? this->_workers.size() : idle));
            }
            for (const bool working: {false, true}) {
                if (working && !mode_forced) break;
                for (auto it = this->_workers.begin(); n > 0 && it != this->_workers.end();) {
                    if (it->second.isWorking() != working) {++it; continue;}
                    this->_idHandler.free(it->first);
                    it = this->_workers.erase(it);
                    --n;
                }
            }
        };
        _cold void spawn(std::size_t n = 1) // spawn n new workers
        {
            std::lock_guard lock(this->_lock);
            if (this->_limit != 0 && this->_workers.size() + n > this->_limit) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds);
            }
            this->_workers.reserve(this->_workers.size() + n);
            for (std::size_t i = 0; i < n; ++i) this->_workers.try_emplace(this->_idHandler.allocate());
        };

        /* getter/setter */
        _cold inline void setLimit(std::size_t limit)                             {std::lock_guard lock(this->_lock); this->_limit = limit;};
        _cold inline void setLifespan(std::chrono::milliseconds lifespan)         {std::lock_guard lock(this->_lock); this->_lifespan = lifespan;};
        _cold _nodiscard inline std::size_t getLimit(void) const                  {std::lock_guard lock(this->_lock); return this->_limit;};
        _cold _nodiscard inline std::chrono::milliseconds getLifespan(void) const {std::lock_guard lock(this->_lock); return this->_lifespan;};
        _cold _nodiscard inline std::size_t size(void) const                      {std::lock_guard lock(this->_lock); return this->_workers.size();};

        // ------------ Operator ---------- //
        LoadBalancer& operator=(const LoadBalancer& other) = delete;
        LoadBalancer& operator=(LoadBalancer&& other) = delete; // the threads keep a pointer on this

        // ---------- Constructor --------- //
        LoadBalancer(std::size_t limit = 1, std::chrono::milliseconds lifespan = std::chrono::milliseconds{0}): _limit{limit}, _lifespan{lifespan} {this->setup_();};
        LoadBalancer(const LoadBalancer& other) = delete;
        LoadBalancer(LoadBalancer&& other) = delete; // the threads keep a pointer on this

        // ----------- Destructor --------- //
        ~LoadBalancer() = default;
};

} // namespace end
#endif /* LOADBALANCER_H */
