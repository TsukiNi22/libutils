/**************************************************************\

  ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
  ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
  ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

  Edition:
##  @date 20/08/2026 by @author Tsukini

File Name:
##  @file Process.cpp

File Description:
##  Definition of the Process's methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/Process.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <vector>

_cold _nodiscard pid_t utils::encapsulation::Process::spawn(void)
{
    if (this->_pid != -1) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Process is already running, call kill() or wait() before spawn()");
    }

    if ((this->_pid = ::fork()) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Kill, ::strerror(errno));
    }

    // child
    if (this->_pid == 0) {
        // Init the pipe & dup
        for (utils::encapsulation::Pipe& pipe: _pipes) pipe.trigger();
        for (utils::encapsulation::Dup& dup: _dups) dup.trigger();

        return this->_pid;
    }

    // parent
    return this->_pid;
}

_cold _nodiscard pid_t utils::encapsulation::Process::spawn(const std::string& path, const std::vector<std::string>& args)
{
    if (this->_pid != -1) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Process is already running, call kill() or wait() before spawn()");
    }

    if ((this->_pid = ::fork()) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Kill, ::strerror(errno));
    }

    // child
    if (this->_pid == 0) {
        // Init the pipe & dup
        for (utils::encapsulation::Pipe& pipe: _pipes) pipe.trigger();
        for (utils::encapsulation::Dup& dup: _dups) dup.trigger();

        // build argv
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(path.c_str()));
        for (const std::string& s: args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);

        // replace this process by the new
        ::execvp(path.c_str(), argv.data());

        // execvp has failed
        ::_exit(127);
    }

    // parent
    return this->_pid;
}

_cold _nodiscard utils::encapsulation::Status utils::encapsulation::Process::wait(void)
{
    if (this->_pid == 0) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Can only be called from the parent, pid: 0");
    } else if (this->_pid == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "The process is already dead, pid: -1");
    }

    int status = 0; // terminated status
    if (::waitpid(this->_pid, &status, 0) == -1) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Waitpid, ::strerror(errno));
    }
    this->_pid = -1;

    // Determine the result
    utils::encapsulation::Status result;
    if (WIFEXITED(status)) { // terminated by exit
        result.exited = true;
        result.code = WEXITSTATUS(status);
        result.sig = 0;
    } else if (WIFSIGNALED(status)) { // terminated by signal
        result.exited = false;
        result.code = 0;
        result.sig = WTERMSIG(status);
    } else { // other case
        result.unknown = true;
        result.exited = false;
        result.code = 0;
        result.sig = 0;
    }

    return result;
}

_cold void utils::encapsulation::Process::kill(void)
{
    if (this->_pid == 0) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Can only be called from the parent, pid: 0");
    } else if (this->_pid == -1) return; // Ignore if already killed

    // ESRCH -> already killed (no existing process+pid)
    if (::kill(this->_pid, SIGKILL) == -1 && errno != ESRCH) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Kill, ::strerror(errno));
    }

    // wait for the end of the process killed
    int status = 0;
    ::waitpid(this->_pid, &status, 0);
    this->_pid = -1;
}
