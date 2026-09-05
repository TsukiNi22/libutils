/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 29/08/2026 by @author Tsukini

File Name:
##  @file Process.hpp

File Description:
##  Process encapsulation class
\**************************************************************/

#ifndef PROCESS_H
    #define PROCESS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"    // utils::security::observer::Observer
    #include "../attribute/Attribute.hpp"           // _cold, _nodiscard
    #include "Pipe.hpp"                             // utils::encapsulation::Pipe
    #include "Dup.hpp"                              // utils::encapsulation::Dup
    #include <unistd.h>                             // pid_t
    #include <vector>                               // std::vector
    #include <string>                               // std::string

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

struct Status {
    bool unknown = false;
    bool exited = false;
    int code = 0; // exited
    int sig = 0; // killed
};

//----------------------------------------------------------------//
/* CLASS */

class Process: private utils::security::observer::Observer<"Process"> {
    private:
        std::vector<utils::encapsulation::Pipe> _pipes;
        std::vector<utils::encapsulation::Dup> _dups;
        pid_t _pid = -1; // -1 not spawned

    public:
        // ---------- Pre-Function -------- //
        /* process */
        bool is(void) const;
        pid_t spawn(void); // fork
        pid_t spawn(const std::string& path, const std::vector<std::string>& args); // execvp
        utils::encapsulation::Status wait(void); // waitpid
        void kill(void); // -s 9

        // ------------ Function ---------- //
        _cold inline void clear(void) {this->_pipes.clear(); this->_dups.clear();}; // clear Pipe/Dup (close fd)

        /* setup */
        _hot inline void pipe(const int read, const int write) {this->_pipes.emplace_back(read, write);};
        _hot inline void dup(const int origin, const int clone = -1) {this->_dups.emplace_back(origin, clone);};
        _hot inline void pipe(utils::encapsulation::Pipe& pipe) {this->_pipes.push_back(std::move(pipe));};
        _hot inline void dup(utils::encapsulation::Dup& dup) {this->_dups.push_back(std::move(dup));};
        _hot inline void pipe(std::vector<utils::encapsulation::Pipe>& pipes) {this->_pipes.insert(this->_pipes.end(), std::make_move_iterator(pipes.begin()), std::make_move_iterator(pipes.end())); pipes.clear();};
        _hot inline void dup(std::vector<utils::encapsulation::Dup>& dups) {this->_dups.insert(this->_dups.end(), std::make_move_iterator(dups.begin()), std::make_move_iterator(dups.end())); dups.clear();};

        /* getter */
        _cold _nodiscard inline std::vector<utils::encapsulation::Pipe>& getPipes(void) {return this->_pipes;};
        _cold _nodiscard inline std::vector<utils::encapsulation::Dup>& getDups(void) {return this->_dups;};
        _cold _nodiscard inline const std::vector<utils::encapsulation::Pipe>& getPipes(void) const {return this->_pipes;};
        _cold _nodiscard inline const std::vector<utils::encapsulation::Dup>& getDups(void) const {return this->_dups;};
        _cold _nodiscard inline bool isChild(void) const {return (this->_pid == 0);};
        _cold _nodiscard inline bool isParent(void) const {return (this->_pid != 0);};
        _cold _nodiscard inline pid_t getPid(void) const {return this->_pid;};

        // ------------ Operator ---------- //
        Process& operator=(const Process& other) = delete;
        Process& operator=(Process&& other) = default;

        // ---------- Constructor --------- //
        Process() = default;
        Process(const Process& other) = delete;
        Process(Process&& other) = default;

        // ----------- Destructor --------- //
        ~Process() {this->kill(); this->clear();};
};

} // namespace end
#endif /* PROCESS_H */
