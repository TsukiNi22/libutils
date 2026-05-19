/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/05/2026 by @author Tsukini

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
utils::cli::ParsedData defaultParserHook(const std::string& input, bool trim, bool logic, bool parse);
bool defaultGetCHook(char& c);

//----------------------------------------------------------------//
/* CLASS */

class Cli: private utils::warning::Observer {
    private:
        /* global data */
        termios _orig;
        bool _sig = false;
        std::atomic<bool> _interrupted = false;
        std::atomic<bool> _running = false;
        std::atomic<std::uint32_t> _flags = utils::cli::Flags::DEFAULT;
        std::atomic<std::uint8_t> _code = 0;
        std::atomic<char> _inputDelimitor = '\n';
        std::queue<std::string> _initInput; // Only at start

        /* storage */
        mutable std::shared_mutex _commandsLock;
        std::unordered_map<std::string, std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>> _parsedCommands;
        std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>> _rawCommands;
        std::shared_mutex _historyLock;
        std::vector<std::string> _history;

        /* hooks */
        mutable std::mutex _hooksLock;
        std::function<void(const utils::cli::Cli&, std::uint8_t)> _promptHook;
        std::function<utils::cli::ParsedData(const std::string&, bool, bool, bool)> _parserHook;
        std::function<bool(char&)> _getcHook;

        // ---------- Pre-Function -------- //
        void launch(std::size_t call); // Stop on ctrl+d (except: MANUAL) or interrupt()
        void prompt();
        std::string getInput();
        utils::cli::ParsedData parse(const std::string& input);
        void exec(const utils::cli::ParsedData& parsedInput);

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
        std::optional<std::thread> start(std::size_t call = 1);
        std::optional<std::thread> start(const std::string& input, std::size_t call = 1); // Execute an input on start
        std::optional<std::thread> start(const std::vector<std::string>& inputs, std::size_t call = 1); // Execute multiple input on start

        /* commands */
        void resetCommands();

        /* hooks */
        void resetHooks(); // Reset all hooks

        /* middlewares */
        void resetMiddlewares(); // Reset all middlewares

        /* getter */
        std::string strcode(std::uint8_t code) const;

        // ------------ Function ---------- //
        void setInputDelimitor(char c) {this->_inputDelimitor = c;};
        void interrupt() {this->_interrupted = true;};
        bool isRunning() const {return this->_running;};

        /* flag */
        void resetFlags() {this->_flags = utils::cli::Flags::DEFAULT;};
        void subFlags(std::uint32_t flags) {this->_flags &= ~flags;};
        void addFlags(std::uint32_t flags) {this->_flags |= flags;};
        void setFlags(std::uint32_t flags) {this->_flags = flags;};

        /* commands */
        void setCommands(const std::unordered_map<std::string, std::tuple<std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>, std::int16_t, std::int16_t>>& commands) {std::unique_lock lock(this->_commandsLock); this->_parsedCommands = commands;}; // Set commands with min & max argument number (-1 = no limit)
        void setCommands(const std::unordered_map<std::string, std::function<void(const utils::cli::Cli&, const std::string&)>>& commands) {std::unique_lock lock(this->_commandsLock); this->_rawCommands = commands;};

        /* hooks */
        void resetPromptHook() {std::lock_guard lock(this->_hooksLock); this->_promptHook = defaultPromptHook;};
        void resetParserHook() {std::lock_guard lock(this->_hooksLock); this->_parserHook = defaultParserHook;};
        void resetGetCHook()   {std::lock_guard lock(this->_hooksLock); this->_getcHook = defaultGetCHook;};
        void setPromptHook(const std::function<void(const utils::cli::Cli&, std::uint8_t)>& hook) {std::lock_guard lock(this->_hooksLock); this->_promptHook = hook;}; // Called to print the prompt
        void setParserHook(const std::function<ParsedData(const std::string&, bool, bool, bool)>& hook) {std::lock_guard lock(this->_hooksLock); this->_parserHook = hook;}; // Called to parse the input
        void setGetCHook(const std::function<bool(char&)>& hook) {std::lock_guard lock(this->_hooksLock); this->_getcHook = hook;}; // Called to print the prompt

        /* getter */
        std::uint8_t getCode() const {return this->_code;};
        std::uint32_t getFlags() const {return this->_flags;};
        char getInputDelimitor() const {return this->_inputDelimitor;};
        std::vector<std::string> getHistory() const {std::lock_guard lock(this->_hooksLock); return this->_history;};

        // ------------ Operator ---------- //
        Cli& operator=(const Cli& other) = delete;
        Cli& operator=(Cli&& other) = delete;

        // ---------- Constructor --------- //
        Cli(bool sig = false); // Enable/Disbale catch of ctrl-c & ctrl-z signal
        Cli(const Cli& other) = delete;
        Cli(Cli&& other) = delete;

        // ----------- Destructor --------- //
        ~Cli();
};

} // namespace end
#endif /* CLI_H */
