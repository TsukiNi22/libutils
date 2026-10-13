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
#include "utils/exception/custom/FatalException.hpp"
#include "utils/encapsulation/Process.hpp"
#include "utils/platform/Platform.hpp"
#ifdef UTILS_WINDOWS
    #include "../platform/Windows.hpp"
#else
    #include <sys/wait.h>
    #include <unistd.h>
    #include <csignal>
#endif
#include <cstring>
#include <cerrno>
#include <vector>
#include <string>

#ifdef UTILS_WINDOWS
    // Quote an argument for the command line (CommandLineToArgvW / CRT rules)
    _cold static std::string quote(const std::string& arg)
    {
        if (!arg.empty() && arg.find_first_of(" \t\n\v\"") == std::string::npos) return arg;

        std::string res = "\"";
        for (std::string::const_iterator it = arg.begin(); ; ++it) {
            std::size_t backslashes = 0;
            while (it != arg.end() && *it == '\\') {++it; ++backslashes;}
            if (it == arg.end()) {
                res.append(backslashes * 2, '\\'); // doubled before the closing quote
                break;
            } else if (*it == '"') {
                res.append(backslashes * 2 + 1, '\\');
                res.push_back('"');
            } else {
                res.append(backslashes, '\\');
                res.push_back(*it);
            }
        }
        res.push_back('"');
        return res;
    }
#endif

_hot _nodiscard bool utils::encapsulation::Process::is(void) const
{
    #ifdef UTILS_WINDOWS
        // Like Unix: the child exists until it's waited/killed (even when it has already exited)
        return this->_pid != -1 && this->_handle != nullptr;
    #else
        // No process (never spawned or already waited/killed), kill(-1, ...) would target every process
        if (this->_pid == -1) return false;

        // Check of existance using dull signal: 0
        if (::kill(this->_pid, 0) == 0) _likely {return true;}
        else if (errno == ESRCH) _unlikely {return false;} // No process with this pid
        else if (errno == EPERM) _likely {return true;} // Process exist but invalid perm
        return false; // Error
    #endif
}

_cold _nodiscard pid_t utils::encapsulation::Process::spawn(void)
{
    #ifdef UTILS_WINDOWS
        throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall, "Process::spawn(void) is a fork: no equivalent on Windows, use spawn(path, args)");
    #else
        if (this->_pid != -1) {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Process is already running, call kill() or wait() before spawn()");
        }

        // Init the pipe
        for (utils::encapsulation::Pipe& pipe: this->_pipes) pipe.trigger();

        if ((this->_pid = ::fork()) == -1) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Fork, ::strerror(errno));
        }

        // child
        if (this->_pid == 0) {
            // Init the dup
            for (utils::encapsulation::Dup& dup: this->_dups) dup.trigger();

            return this->_pid;
        }

        // parent
        return this->_pid;
    #endif
}

_cold _nodiscard pid_t utils::encapsulation::Process::spawn(const std::string& path, const std::vector<std::string>& args)
{
    #ifdef UTILS_WINDOWS
        if (this->_pid != -1) {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Process is already running, call kill() or wait() before spawn()");
        }

        // Check the dup before the creation: only the standard fds can be given to the child
        for (const utils::encapsulation::Dup& dup: this->_dups) {
            if (dup.getOrigin() == -1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Dup, "The origin fd is close, please set a valid fd: setOrigin(int fd)");
            } else if (dup.getClone() < 0 || dup.getClone() > 2) _unlikely {
                throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall, "Only the fds 0, 1 & 2 of the child can be redirected on Windows, got: " + std::to_string(dup.getClone()));
            }
        }

        // Init the pipe
        for (utils::encapsulation::Pipe& pipe: this->_pipes) pipe.trigger();

        // Standard handles of the child: the dup (last one wins, like dup2) or the ones of the parent
        HANDLE handles[3] = {::GetStdHandle(STD_INPUT_HANDLE), ::GetStdHandle(STD_OUTPUT_HANDLE), ::GetStdHandle(STD_ERROR_HANDLE)};
        for (const utils::encapsulation::Dup& dup: this->_dups) {
            const HANDLE handle = reinterpret_cast<HANDLE>(::_get_osfhandle(dup.getOrigin()));
            if (handle == INVALID_HANDLE_VALUE) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Dup, "Invalid origin fd: " + std::to_string(dup.getOrigin()));
            }
            handles[dup.getClone()] = handle;
        }

        // Inheritable copies, the only handles given to the child (no leak of the other pipes: EOF stays reachable)
        HANDLE inherited[3] = {nullptr, nullptr, nullptr};
        std::vector<HANDLE> list;
        for (int i = 0; i < 3; ++i) {
            if (!handles[i] || handles[i] == INVALID_HANDLE_VALUE) continue;
            if (::DuplicateHandle(::GetCurrentProcess(), handles[i], ::GetCurrentProcess(), &inherited[i], 0, TRUE, DUPLICATE_SAME_ACCESS))
                list.push_back(inherited[i]);
        }

        // Attribute list restricting the inheritance
        SIZE_T size = 0;
        (void)::InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
        std::vector<char> attributes(size);
        LPPROC_THREAD_ATTRIBUTE_LIST attributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributes.data());
        if (!::InitializeProcThreadAttributeList(attributeList, 1, 0, &size)
            || (!list.empty() && !::UpdateProcThreadAttribute(attributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, list.data(), list.size() * sizeof(HANDLE), nullptr, nullptr))) _unlikely {
            for (HANDLE handle: inherited) if (handle) ::CloseHandle(handle);
            throw utils::exception::ErrorException(utils::exception::InternalCode::Fork, utils::platform::windows::strerror(::GetLastError()));
        }

        // Command line (searched in the PATH like execvp)
        std::string cmd = quote(path);
        for (const std::string& arg: args) cmd += " " + quote(arg);

        STARTUPINFOEXA info{};
        info.StartupInfo.cb = sizeof(info);
        info.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
        info.StartupInfo.hStdInput = inherited[0];
        info.StartupInfo.hStdOutput = inherited[1];
        info.StartupInfo.hStdError = inherited[2];
        info.lpAttributeList = attributeList;

        PROCESS_INFORMATION process{};
        const BOOL created = ::CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, !list.empty(), EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, &info.StartupInfo, &process);
        const std::string error = created ? "" : utils::platform::windows::strerror(::GetLastError());
        ::DeleteProcThreadAttributeList(attributeList);
        for (HANDLE handle: inherited) if (handle) ::CloseHandle(handle); // the child has its own copies
        if (!created) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Fork, error);
        }

        ::CloseHandle(process.hThread);
        this->_handle = process.hProcess;
        this->_pid = static_cast<pid_t>(process.dwProcessId);
        return this->_pid;
    #else
        if (this->_pid != -1) {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Process is already running, call kill() or wait() before spawn()");
        }

        // Check the dup before the fork (an error in the child can't reach the caller)
        for (const utils::encapsulation::Dup& dup: this->_dups) {
            if (dup.getOrigin() == -1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Dup, "The origin fd is close, please set a valid fd: setOrigin(int fd)");
            }
        }

        // build argv (no allocation between the fork & the exec)
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(path.c_str()));
        for (const std::string& s: args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);

        // Init the pipe
        for (utils::encapsulation::Pipe& pipe: this->_pipes) pipe.trigger();

        if ((this->_pid = ::fork()) == -1) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Fork, ::strerror(errno));
        }

        // child
        if (this->_pid == 0) {
            // Init the dup (the child must never go back to the caller's code)
            try {
                for (utils::encapsulation::Dup& dup: this->_dups) dup.trigger();
            } catch (...) {
                ::_exit(127);
            }

            // replace this process by the new
            ::execvp(path.c_str(), argv.data());

            // execvp has failed
            ::_exit(127);
        }

        // parent
        return this->_pid;
    #endif
}

_cold _noreturn void utils::encapsulation::Process::replace(const std::string& path, const std::vector<std::string>& args)
{
    #ifdef UTILS_WINDOWS
        if (this->_pid != -1) {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Process is already running, call kill() or wait() before spawn()");
        }

        // Init the pipe & dup
        for (utils::encapsulation::Pipe& pipe: this->_pipes) pipe.trigger();
        for (utils::encapsulation::Dup& dup: this->_dups) dup.trigger();

        // build argv (quoted: the CRT joins them in a command line)
        std::vector<std::string> quoted;
        quoted.push_back(quote(path));
        for (const std::string& s: args) quoted.push_back(quote(s));
        std::vector<const char*> argv;
        for (const std::string& s: quoted) argv.push_back(s.c_str());
        argv.push_back(nullptr);

        // replace this process by the new (CRT: start the new process then exit)
        ::_execvp(path.c_str(), argv.data());

        // _execvp has failed
        ::_exit(127);
    #else
        if (this->_pid != -1) {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Process is already running, call kill() or wait() before spawn()");
        }

        // Init the pipe & dup
        for (utils::encapsulation::Pipe& pipe: this->_pipes) pipe.trigger();
        for (utils::encapsulation::Dup& dup: this->_dups) dup.trigger();

        // build argv
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(path.c_str()));
        for (const std::string& s: args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);

        // replace this process by the new
        ::execvp(path.c_str(), argv.data());

        // execvp has failed
        ::_exit(127);
    #endif
}

_cold _nodiscard utils::encapsulation::Status utils::encapsulation::Process::wait(void)
{
    #ifdef UTILS_WINDOWS
        if (this->_pid == -1 || !this->_handle) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "The process is already dead, pid: -1");
        }

        // Wait the end & get the exit code
        DWORD code = 0;
        if (::WaitForSingleObject(this->_handle, INFINITE) == WAIT_FAILED || !::GetExitCodeProcess(this->_handle, &code)) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Waitpid, utils::platform::windows::strerror(::GetLastError()));
        }
        ::CloseHandle(this->_handle);
        this->_handle = nullptr;
        this->_pid = -1;

        // No signal on Windows: always an exit code
        utils::encapsulation::Status result;
        result.exited = true;
        result.code = static_cast<int>(code);
        return result;
    #else
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
    #endif
}

_cold void utils::encapsulation::Process::kill(void)
{
    #ifdef UTILS_WINDOWS
        if (this->_pid == -1 || !this->_handle) return; // Ignore if already killed

        // Already exited: TerminateProcess fails (access denied), nothing to kill
        if (!::TerminateProcess(this->_handle, 137) && ::WaitForSingleObject(this->_handle, 0) != WAIT_OBJECT_0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Kill, utils::platform::windows::strerror(::GetLastError()));
        }

        // wait for the end of the process killed
        (void)::WaitForSingleObject(this->_handle, INFINITE);
        ::CloseHandle(this->_handle);
        this->_handle = nullptr;
        this->_pid = -1;
    #else
        if (this->_pid == 0) {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Can only be called from the parent, pid: 0");
        } else if (this->_pid == -1) return; // Ignore if already killed

        // ESRCH -> already killed (no existing process+pid)
        if (::kill(this->_pid, SIGKILL) == -1 && errno != ESRCH) {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Kill, ::strerror(errno));
        }

        // wait for the end of the process killed
        int status = 0;
        (void)::waitpid(this->_pid, &status, 0);
        this->_pid = -1;
    #endif
}
