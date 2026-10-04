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
##  @file Cli.hpp

File Description:
##  Cli class used for a customizable command line interface
\**************************************************************/

#ifndef CLI_H
    #define CLI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../security/observer/Observer.hpp"        // utils::security::observer::Observer
    #include "../pool/middleware/Middlewares.hpp"       // utils::pool::Middlewares
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::InternalCode
    #include "../attribute/Attribute.hpp"               // _cold, _nodiscard
    #include "Flags.hpp"                                // utils::cli::Flag, utils::cli::flags::* (preset)
    #include <termios.h>                                // termios
    #include <csignal>                                  // SIG_DFL
    #include <unordered_map>                            // std::unordered_map
    #include <shared_mutex>                             // std::shared_mutex, std::unique_lock, std::shared_lock
    #include <functional>                               // std::function
    #include <optional>                                 // std::optional
    #include <cstddef>                                  // std::size_t
    #include <cstdint>                                  // std::uint8_t, std::int16_t, std::uint32_t
    #include <thread>                                   // std::thread
    #include <atomic>                                   // std::atomic
    #include <vector>                                   // std::vector
    #include <string>                                   // std::string
    #include <mutex>                                    // std::mutex, std::lock_guard
    #include <tuple>                                    // std::tuple
    #include <queue>                                    // std::queue

    //----------------------------------------------------------------//
    /* DEFINE */

    /* values */
    #define HISTORY_FILE ".utils-cli_history"
    #define HISTORY_LIMITS 10000

namespace utils::cli { // namespace start
//----------------------------------------------------------------//
/* TYPE */

// Return of the parser
using ParsedData =
std::vector< // Group of commands, separated by '&&', '||' and ';'
    std::vector< // Command separated by ' ' and '\t'
        // the first std::string represent only the command
        // The input parsed (depends on the parser mode)
        // the last std::string represent the separator '&&', '||', ';' or '' for nothing/last
        std::string
    >
>;

//----------------------------------------------------------------//
/* PROTOTYPE */

class Cli;

/* default hooks */
void defaultPromptHook(const utils::cli::Cli& cli, std::uint8_t code);
utils::cli::ParsedData defaultParserHook(const std::string& input, const bool trim, const bool logic, const bool parse);
bool defaultGetCHook(char& c);

//----------------------------------------------------------------//
/* CLASS */

class Cli: private utils::security::observer::Observer<"Cli"> {
    private:
        /* global data */
        termios _orig;
        bool _termios = false; // terminal setup done (to restore)
        bool _sig = false;
        void (*_oldSigInt)(int) = SIG_DFL; // handlers before the cli (restored at the destruction)
        void (*_oldSigTstp)(int) = SIG_DFL;
        std::atomic<bool> _interrupted = false;
        std::atomic<bool> _killed = false; // Can't be undone
        std::atomic<bool> _running = false;
        std::atomic<std::uint32_t> _flags = utils::cli::flags::DEFAULT;
        std::atomic<std::uint8_t> _code = 0;
        std::atomic<char> _inputDelimitor = '\n';
        std::queue<std::string> _initInput; // Only at start

        /* storage */
        std::string _input; // Internal storage for input buffer used on interrupt & restart
        mutable std::shared_mutex _commandsLock;
        std::unordered_map<std::string, std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>> _parsedCommands;
        std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>> _rawCommands;
        mutable std::shared_mutex _historyLock;
        std::vector<std::string> _history;

        /* hooks */
        mutable std::mutex _hooksLock;
        std::function<void(const utils::cli::Cli&, std::uint8_t)> _promptHook;
        std::function<utils::cli::ParsedData(const std::string&, const bool, const bool, const bool)> _parserHook;
        std::function<bool(char&)> _getcHook;

        // ---------- Pre-Function -------- //
        void launch_(const std::size_t call); // Stop on ctrl+d (except: MANUAL) or interrupt()
        void prompt_(void);
        std::string getInput_(void);
        utils::cli::ParsedData parse_(const std::string& input);
        void exec_(const utils::cli::ParsedData& parsedInput);

        /* persistent storage handling */
        void loadHistory_(void);
        void saveHistory_(void);

    public:
        /* middlewares */
        utils::pool::Middlewares<void, void> cliMiddlewares; // When the cli start & end
        utils::pool::Middlewares<std::uint8_t, std::uint8_t> errorMiddlewares; // When an error is triggered
        utils::pool::Middlewares<void, void> promptMiddlewares; // When the prompt is displayed
        utils::pool::Middlewares<void, char> inputMiddlewares; // When a key is pressed (only after is used)
        utils::pool::Middlewares<const std::string&, const utils::cli::ParsedData&> parserMiddlewares; // When the parser is called
        utils::pool::Middlewares<const utils::cli::ParsedData&, const utils::cli::ParsedData&> execMiddlewares; // When the parsed data is executed
        utils::pool::Middlewares<const std::string&, const std::string&> commandMiddlewares; // When a command is executed

        // ---------- Pre-Function -------- //
        void join(void) const noexcept; // Yield until the cli stop running
        std::optional<std::thread> start(const std::size_t call = 1, const bool failsafe = false);
        std::optional<std::thread> start(const std::string& input, const std::size_t call = 1, const bool failsafe = false); // Execute an input on start
        std::optional<std::thread> start(const std::vector<std::string>& inputs, const std::size_t call = 1, const bool failsafe = false); // Execute multiple input on start

        /* commands */
        void resetCommands(void);
        void clearCommands(void);
        void delCommand(const std::string& command);
        void delCommands(const std::vector<std::string>& commands);

        /* hooks */
        void resetHooks(void); // Reset all hooks

        /* middlewares */
        void resetMiddlewares(void); // Reset all middlewares

        /* getter */
        std::string strcode(std::uint8_t code) const;

        // ------------ Function ---------- //
        _cold inline void setInputDelimitor(const char c) {this->_inputDelimitor = c;};
        _cold inline void interrupt(void)                 {this->_interrupted = true;};
        _cold inline void kill(void)                      {this->_killed = true; this->_interrupted = true;};

        /* status */
        _cold _nodiscard inline bool isRunning(void) const      {return this->_running;};
        _cold _nodiscard inline bool wasInterrupted(void) const {return (!this->_running && this->_interrupted && !this->_killed);};
        _cold _nodiscard inline bool wasKilled(void) const      {return (!this->_running && this->_killed);};
        _cold _nodiscard inline bool wasStopped(void) const     {return (!this->_running && !this->_interrupted && !this->_killed);};

        /* flag */
        _cold inline void resetFlags(void)              {this->_flags = utils::cli::flags::DEFAULT;};
        _cold inline void subFlags(std::uint32_t flags) {this->_flags &= ~flags;};
        _cold inline void addFlags(std::uint32_t flags) {this->_flags |= flags;};
        _cold inline void setFlags(std::uint32_t flags) {this->_flags = flags;};

        /* commands */
        template<bool force = false> // Can't override an exiting one by default, throw of error
        _cold void setCommand(const std::string& command, const std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>& tup)
        {
            std::unique_lock lock(this->_commandsLock);
            if constexpr (!force) {
                if (this->_parsedCommands.contains(command))
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("This command is already defined (parsed): ") + command);
            }
            this->_parsedCommands[command] = tup;
        };
        template<bool force = false> // Can't override an exiting one by default, throw of error
        _cold void setCommand(const std::string& command, const std::function<void(const utils::cli::Cli&, const std::string&)>& fn)
        {
            std::unique_lock lock(this->_commandsLock);
            if constexpr (!force) {
                if (this->_rawCommands.contains(command))
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("This command is already defined (raw): ") + command);
            }
            this->_rawCommands[command] = fn;
        };
        template<bool force = false> // Can't override an exiting one by default, throw of error
        _cold void setCommands(const std::unordered_map<std::string, std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>>& commands)
        {
            std::unique_lock lock(this->_commandsLock);
            for (const auto &[command, tup]: commands) {
                if constexpr (!force) {
                    if (this->_parsedCommands.contains(command))
                        throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("This command is already defined (parsed): ") + command);
                }
                this->_parsedCommands[command] = tup;
            }
        };
        template<bool force = false> // Can't override an exiting one by default, throw of error
        _cold void setCommands(const std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>>& commands)
        {
            std::unique_lock lock(this->_commandsLock);
            for (const auto &[command, fn]: commands) {
                if constexpr (!force) {
                    if (this->_rawCommands.contains(command))
                        throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("This command is already defined (raw): ") + command);
                }
                this->_rawCommands[command] = fn;
            }
        };

        /* hooks */
        _cold inline void resetPromptHook(void)                                                                                  {std::lock_guard lock(this->_hooksLock); this->_promptHook = utils::cli::defaultPromptHook;};
        _cold inline void resetParserHook(void)                                                                                  {std::lock_guard lock(this->_hooksLock); this->_parserHook = utils::cli::defaultParserHook;};
        _cold inline void resetGetCHook(void)                                                                                    {std::lock_guard lock(this->_hooksLock); this->_getcHook = utils::cli::defaultGetCHook;};
        _cold inline void setPromptHook(const std::function<void(const utils::cli::Cli&, std::uint8_t)>& hook)                   {std::lock_guard lock(this->_hooksLock); this->_promptHook = hook;}; // Called to print the prompt
        _cold inline void setParserHook(const std::function<utils::cli::ParsedData(const std::string&, bool, bool, bool)>& hook) {std::lock_guard lock(this->_hooksLock); this->_parserHook = hook;}; // Called to parse the input
        _cold inline void setGetCHook(const std::function<bool(char&)>& hook)                                                    {std::lock_guard lock(this->_hooksLock); this->_getcHook = hook;}; // Called to get a char of the input

        /* getter */
        _cold _nodiscard inline std::uint8_t getCode(void) const                {return this->_code;};
        _cold _nodiscard inline std::uint32_t getFlags(void) const              {return this->_flags;};
        _cold _nodiscard inline char getInputDelimitor(void) const              {return this->_inputDelimitor;};
        _cold _nodiscard inline std::vector<std::string> getHistory(void) const {std::shared_lock lock(this->_historyLock); return this->_history;};

        // ------------ Operator ---------- //
        Cli& operator=(const Cli& other) = delete;
        Cli& operator=(Cli&& other) = delete;

        // ---------- Constructor --------- //
        Cli(const bool sig = false); // Enable/Disable catch of ctrl-c & ctrl-z signal
        Cli(const Cli& other) = delete;
        Cli(Cli&& other) = delete;

        // ----------- Destructor --------- //
        ~Cli();
};

} // namespace end
#endif /* CLI_H */
