/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 06/07/2026 by @author Tsukini

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
    #include "../warning/Observer.hpp"          // utils::warning::Observer
    #include "../middleware/Middlewares.hpp"    // utils::middleware::Middlewares
    #include "Flags.hpp"                        // utils::cli::Flag, flag preset macro
    #include <termios.h>                        // termios
    #include <unordered_map>                    // std::unordered_map
    #include <shared_mutex>                     // std::shared_mutex, std::unique_lock, std::shared_lock
    #include <functional>                       // std::function
    #include <optional>                         // std::optional
    #include <cstdint>                          // std::uint8_t, std::uint32_t
    #include <thread>                           // std::thread
    #include <atomic>                           // std::atomic
    #include <vector>                           // std::vector
    #include <string>                           // std::string
    #include <mutex>                            // std::mutex, std::lock_guard
    #include <tuple>                            // std::tuple
    #include <queue>                            // std::queue

    //----------------------------------------------------------------//
    /* DEFINE */

    /* values */
    #define HISTORY_FILE ".utils-cli_history"
    #define HISTORY_LIMITS 10000

namespace utils::cli { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

// Return of the parser
using ParsedData =
std::vector< // Groupe of commands, separated by '&&', '||' and ';'
    std::vector< // Command separated by ' ' and '\t'
        // the first std::string represent only the command
        // The input parsed (depende on the parser mode)
        // the last std::string represent the separator '&&', '||', ';' or '' for nothing/last
        std::string
    >
>;

class Cli;

/* default hooks */
void defaultPromptHook(const utils::cli::Cli& cli, std::uint8_t code);
utils::cli::ParsedData defaultParserHook(const std::string& input, const bool trim, const bool logic, const bool parse);
bool defaultGetCHook(char& c);

//----------------------------------------------------------------//
/* CLASS */

class Cli: private utils::warning::Observer {
    private:
        /* global data */
        termios _orig;
        bool _sig = false;
        std::atomic<bool> _interrupted = false;
        std::atomic<bool> _killed = false; // Can't be undone
        std::atomic<bool> _running = false;
        std::atomic<std::uint32_t> _flags = utils::cli::Flags::DEFAULT;
        std::atomic<std::uint8_t> _code = 0;
        std::atomic<char> _inputDelimitor = '\n';
        std::queue<std::string> _initInput; // Only at start

        /* storage */
        std::string _input; // Internal storage for input buffer used on interrupt & restart
        mutable std::shared_mutex _commandsLock;
        std::unordered_map<std::string, std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>> _parsedCommands;
        std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>> _rawCommands;
        std::shared_mutex _historyLock;
        std::vector<std::string> _history;

        /* hooks */
        mutable std::mutex _hooksLock;
        std::function<void(const utils::cli::Cli&, std::uint8_t)> _promptHook;
        std::function<utils::cli::ParsedData(const std::string&, const bool, const bool, const bool)> _parserHook;
        std::function<bool(char&)> _getcHook;

        // ---------- Pre-Function -------- //
        void launch(const std::size_t call); // Stop on ctrl+d (except: MANUAL) or interrupt()
        void prompt(void);
        std::string getInput(void);
        utils::cli::ParsedData parse(const std::string& input);
        void exec(const utils::cli::ParsedData& parsedInput);

        /* persitent storage handling */
        void loadHistory(void);
        void saveHistory(void);

    public:
        /* middlewares */
        utils::middleware::Middlewares<void, void> cliMiddlewares; // When the cli start & end
        utils::middleware::Middlewares<std::uint8_t, std::uint8_t> errorMiddlewares; // When an error is triggered
        utils::middleware::Middlewares<void, void> promptMiddlewares; // When the prompt is displayed
        utils::middleware::Middlewares<void, char> inputMiddlewares; // When a key is pressed (only after is used)
        utils::middleware::Middlewares<const std::string&, const utils::cli::ParsedData&> parserMiddlewares; // When the parser is called
        utils::middleware::Middlewares<const utils::cli::ParsedData&, const utils::cli::ParsedData&> execMiddlewares; // When the parsed data is executed
        utils::middleware::Middlewares<const std::string&, const std::string&> commandMiddlewares; // When a command is executed

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
        void setInputDelimitor(const char c) {this->_inputDelimitor = c;};
        void interrupt(void) {this->_interrupted = true;};
        void kill(void) {this->_killed = true; this->_interrupted = true;};
        bool isRunning(void) const {return this->_running;};
        bool wasInterrupted(void) const {return (!this->_running && this->_interrupted && !this->_killed);};
        bool wasKilled(void) const {return (!this->_running && this->_killed);};
        bool wasStopped(void) const {return (!this->_running && !this->_interrupted && !this->_killed);};

        /* flag */
        void resetFlags(void) {this->_flags = utils::cli::Flags::DEFAULT;};
        void subFlags(std::uint32_t flags) {this->_flags &= ~flags;};
        void addFlags(std::uint32_t flags) {this->_flags |= flags;};
        void setFlags(std::uint32_t flags) {this->_flags = flags;};

        /* commands */
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setCommand(const std::string& command, const std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>& tup)
        {
            std::unique_lock lock(this->_commandsLock);
            if constexpr (!force) {
                if (this->_parsedCommands.contains(command))
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("This command is already defined (parsed): ") + command);
            }
            this->_parsedCommands[command] = tup;
        };
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setCommand(const std::string& command, const std::function<void(const utils::cli::Cli&, const std::string&)>& fn)
        {
            std::unique_lock lock(this->_commandsLock);
            if constexpr (!force) {
                if (this->_rawCommands.contains(command))
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("This command is already defined (raw): ") + command);
            }
            this->_rawCommands[command] = fn;
        };
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setCommands(const std::unordered_map<std::string, std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>>& commands)
        {
            std::unique_lock lock(this->_commandsLock);
            for (const auto&[command, tup]: commands) {
                if constexpr (!force) {
                    if (this->_parsedCommands.contains(command))
                        throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("This command is already defined (parsed): ") + command);
                }
                this->_parsedCommands[command] = tup;
            }
        };
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setCommands(const std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>>& commands)
        {
            std::unique_lock lock(this->_commandsLock);
            for (const auto&[command, fn]: commands) {
                if constexpr (!force) {
                    if (this->_rawCommands.contains(command))
                        throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("This command is already defined (raw): ") + command);
                }
                this->_rawCommands[command] = fn;
            }
        };

        /* hooks */
        void resetPromptHook(void) {std::lock_guard lock(this->_hooksLock); this->_promptHook = defaultPromptHook;};
        void resetParserHook(void) {std::lock_guard lock(this->_hooksLock); this->_parserHook = defaultParserHook;};
        void resetGetCHook(void)   {std::lock_guard lock(this->_hooksLock); this->_getcHook = defaultGetCHook;};
        void setPromptHook(const std::function<void(const utils::cli::Cli&, std::uint8_t)>& hook) {std::lock_guard lock(this->_hooksLock); this->_promptHook = hook;}; // Called to print the prompt
        void setParserHook(const std::function<ParsedData(const std::string&, bool, bool, bool)>& hook) {std::lock_guard lock(this->_hooksLock); this->_parserHook = hook;}; // Called to parse the input
        void setGetCHook(const std::function<bool(char&)>& hook) {std::lock_guard lock(this->_hooksLock); this->_getcHook = hook;}; // Called to print the prompt

        /* getter */
        std::uint8_t getCode(void) const {return this->_code;};
        std::uint32_t getFlags(void) const {return this->_flags;};
        char getInputDelimitor(void) const {return this->_inputDelimitor;};
        std::vector<std::string> getHistory() const {std::lock_guard lock(this->_hooksLock); return this->_history;};

        // ------------ Operator ---------- //
        Cli& operator=(const Cli& other) = delete;
        Cli& operator=(Cli&& other) = delete;

        // ---------- Constructor --------- //
        Cli(const bool sig = false); // Enable/Disbale catch of ctrl-c & ctrl-z signal
        Cli(const Cli& other) = delete;
        Cli(Cli&& other) = delete;

        // ----------- Destructor --------- //
        ~Cli();
};

} // namespace end
#endif /* CLI_H */
