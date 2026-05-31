/**************************************************************\
Edition:
##  @date 29/05/2026 by @author Tsukini

File Name:
##  @file ArgParser.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef ARGPARSER_H
    #define ARGPARSER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/custom/CustomException.hpp"  // utils::exception::CustomException
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::* (Type)
    #include <unordered_map>                            // std::unordered_map
    #include <functional>                               // std::function
    #include <vector>                                   // std::vector
    #include <string>                                   // std::string
    #include <tuple>                                    // std::tuple

namespace utils::arguments { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

// Return of the parser
using ParsedData = std::unordered_map<
    std::string, // id
    std::vector<std::string> // options
>;

struct Usage {
    std::string name;
    bool ordered = false; // The order of the option is mandatory
    std::vector<std::pair<std::string, bool>> ids; // List of ids allowed by this usage <id, mandatory>
    std::string description = "[None]";
};

struct Option {
    std::string name;
    std::function<bool(const std::string&)> check;
    std::string description = "[None]";
};

struct Flag {
    std::tuple<std::string> flag; // <short, flag, long>
    std::vector<std::function<bool(const std::string&)>> checks;
    std::string description = "[None]";
};

class ArgParser;

/* default hooks (parsing = only check) */
void defaultHelpHook(const utils::arguments::ArgParser& parser);
bool defaultBoolParsingHook(const std::string& option);     // Parse boolean (0, 1, fase, true)
bool defaultInt32ParsingHook(const std::string& option);    // Parse std::int32_t
bool defaultSizetParsingHook(const std::string& option);    // Parse std::size_t
bool defaultDoubleParsingHook(const std::string& option);   // Parse double
bool defaultFileParsingHook(const std::string& option);     // Check for file reading (only!)
bool defaultWritableParsingHook(const std::string& option); // Check if the path/file is readable & writable (only!)

//----------------------------------------------------------------//
/* CLASS */

class ArgParser {
    private:
        std::unordered_map<std::string, utils::arguments::Usage> _usages;
        std::unordered_map<std::string, utils::arguments::Flag> _flags;
        std::unordered_map<std::string, utils::arguments::Option> _options;
        std::function<void(const utils::arguments::ArgParser& parser)> _helpHook;

    public:
        // ---------- Pre-Function -------- //
        void help(void); // Help display (using hook)

        /* parsing */
        // Allways ignore the first argument, return a list of flag's found
        // Check only the option & dosn't
        utils::arguments::ParsedData parse(const int argc, const char *const argv[], bool failsafe = false);
        utils::arguments::ParsedData parse(std::vector<std::string>& argv, bool failsafe = false);

        /* setup */
        void delUsage(const std::string& id);
        void delUsages(const std::vector<std::string>& ids);
        void delOption(const std::string& id);
        void delOptions(const std::vector<std::string>& ids);
        void delFlag(const std::string& id);
        void delFlags(const std::vector<std::string>& ids);

        /* getter */
        std::string getDescription(const std::string& id);

        // ------------ Function ---------- //
         /* hook handling */
        void setHelpHook(std::function<void(const utils::arguments::ArgParser& parser)> hook) {this->_helpHook = hook;};
        void resetHelpHook(void) {this->_helpHook = defaultHelpHook;};

        /* setup */
        template<bool overwrite = false> // Can't override an exiting one by default, throw of error
        void setUsage(const std::string& id, const std::string& name, const bool ordered, const std::vector<std::pair<std::string, bool>>& ids, const std::string& description = "[None]")
        {
            if constexpr (!overwrite) {
                if (this->_usages.contains(id))
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::Override, std::string("An option with this id is already defined: ") + id);
            }
            this->_usages[id] = utils::arguments::Usage{name, ordered, ids, description};
        };
        void resetUsages(void) {this->_usages.clear();};
        template<bool overwrite = false> // Can't override an exiting one by default, throw of error
        void setOption(const std::string& id, const std::string& name, std::function<bool(const std::string&)> check, const std::string& description = "[None]")
        {
            if constexpr (!overwrite) {
                if (this->_options.contains(id))
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::Override, std::string("An option with this id is already defined: ") + id);
            }
            this->_options[id] = utils::arguments::Option{name, check, description};
        };
        void resetOptions(void) {this->_options.clear();};
        template<bool overwrite = false> // Can't override an exiting one by default, throw of error
        void setFlag(const std::string& id, const std::tuple<std::string>& flag, const std::vector<std::function<bool(const std::string&)>>& checks, const std::string& description = "[None]")
        {
            if constexpr (!overwrite) {
                if (this->_flags.contains(id))
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::Override, std::string("An flag with this id is already defined: ") + id);
            }
            this->_flags[id] = utils::arguments::Flag{flag, checks, description};
        };
        void resetFlags(void) {this->_flags.clear();};

        // ------------ Operator ---------- //
        ArgParser& operator=(const ArgParser& other) = delete;
        ArgParser& operator=(ArgParser&& other) = delete;

        // ---------- Constructor --------- //
        ArgParser();
        ArgParser(const ArgParser& other) = delete;
        ArgParser(ArgParser&& other) = delete;

        // ----------- Destructor --------- //
        ~ArgParser() = default;
};

} // namespace end
#endif /* ARGPARSER_H */
