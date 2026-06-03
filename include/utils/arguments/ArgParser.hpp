/**************************************************************\
Edition:
##  @date 03/06/2026 by @author Tsukini

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
    #define NO_OUTDATED_WARNING
    #include "../attribute/Attribute.hpp"               // nodicard, unused
    #include "../warning/Observer.hpp"                  // utils::warning::Observer
    #include "../exception/custom/CustomException.hpp"  // utils::exception::CustomException
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::* (Type)
    #include "ArgParserType.hpp"                        // utils::arguments::* (Type)
    #include <unordered_map>                            // std::unordered_map
    #include <functional>                               // std::function
    #include <vector>                                   // std::vector
    #include <string>                                   // std::string
    #include <tuple>                                    // std::tuple

namespace utils::arguments { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

class ArgParser;

/* default hooks (parsing = only check) */
void defaultHelpHook(const utils::arguments::ArgParser& parser);
bool defaultBoolParsingHook(const std::string& option);     // Parse boolean (0, 1, fase, true)
bool defaultInt32ParsingHook(const std::string& option);    // Parse std::int32_t
bool defaultSizetParsingHook(const std::string& option);    // Parse std::size_t
bool defaultDoubleParsingHook(const std::string& option);   // Parse double
bool defaultFileParsingHook(const std::string& option);     // Check for file reading (only!)
bool defaultWritableParsingHook(const std::string& option); // Check if the path/file is readable & writable (only!)
inline bool defaultTrueParsingHook(unused const std::string&) {return true;};

//----------------------------------------------------------------//
/* CLASS */

class ArgParser: private utils::warning::Observer {
    private:
        std::string _binary = "[None]";
        std::string _description = "...";
        std::unordered_map<std::string, utils::arguments::Usage> _usages;
        std::unordered_map<std::string, utils::arguments::Option> _options;
        std::unordered_map<std::string, utils::arguments::Flag> _flags;
        std::function<void(const utils::arguments::ArgParser& parser)> _helpHook;

        // ---------- Pre-Function -------- //
        /* sub parsing */
        void parseFlags(utils::arguments::ParsedData& data, const std::vector<std::string>& argv, std::size_t& i, const bool failsafe = false);
        void parseOption(utils::arguments::ParsedData& data, const std::vector<std::string>& argv, std::size_t& i, const bool failsafe = false);

    public:
        // ---------- Pre-Function -------- //
        void help(void); // Help display (using hook)

        /* parsing */
        // Allways ignore the first argument, return a list of flag's found
        // Check only the option & dosn't
        utils::arguments::ParsedData parse(const int argc, const char *const argv[], const bool failsafe = false);
        utils::arguments::ParsedData parse(const std::vector<std::string>& argv, const bool failsafe = false);

        /* setup */
        void delUsage(const std::string& id);
        void delUsages(const std::vector<std::string>& ids);
        void delOption(const std::string& id);
        void delOptions(const std::vector<std::string>& ids);
        void delFlag(const std::string& id);
        void delFlags(const std::vector<std::string>& ids);

        // ------------ Function ---------- //
        /* setup */
        void setDefaultUsage(void) {this->_usages["default"] = utils::arguments::Usage{"default", false, {}, "Default usage with all flag(s) & option(s)"};};
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setUsage(const std::string& id, const std::string& name, const bool ordered, const std::vector<std::pair<std::string, bool>>& ids, const std::string& description = "[None]")
        {
            if constexpr (!force) {
                if (this->_usages.contains(id))
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::Override, std::string("An option with this id is already defined: ") + id);
            }
            this->_usages[id] = utils::arguments::Usage{name, ordered, ids, description};
        };
        void resetUsages(void) {this->_usages.clear();};
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setOption(const std::string& id, const std::string& name, std::function<bool(const std::string&)> check, const std::string& description = "[None]")
        {
            if constexpr (!force) {
                if (this->_options.contains(id))
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::Override, std::string("An option with this id is already defined: ") + id);
            }
            this->_options[id] = utils::arguments::Option{name, check, description};
        };
        void resetOptions(void) {this->_options.clear();};
        template<bool force = false> // Can't override an exiting one by default, throw of error
        void setFlag(const std::string& id, const std::tuple<std::string, std::string, std::string>& flag, const std::vector<std::tuple<std::string, bool, std::function<bool(const std::string&)>>>& options, const std::string& description = "[None]")
        {
            if constexpr (!force) {
                if (this->_flags.contains(id))
                    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::Override, std::string("An flag with this id is already defined: ") + id);
            }
            this->_flags[id] = utils::arguments::Flag{flag, options, description};
        };
        void resetFlags(void) {this->_flags.clear();};

        /* hook handling */
        void setHelpHook(std::function<void(const utils::arguments::ArgParser& parser)> hook) {this->_helpHook = hook;};
        void resetHelpHook(void) {this->_helpHook = defaultHelpHook;};

        /* setter */
        void setBinary(const std::string& binary) {this->_binary = binary;};
        void setDescription(const std::string& description) {this->_description = description;};

        /* getter */
        nodiscard const std::string& getBinary(void) const {return this->_binary;};
        nodiscard const std::string& getDescription(void) const {return this->_description;};
        nodiscard const std::unordered_map<std::string, utils::arguments::Usage>& getUsages(void) const {return this->_usages;};
        nodiscard const std::unordered_map<std::string, utils::arguments::Option>& getOptions(void) const {return this->_options;};
        nodiscard const std::unordered_map<std::string, utils::arguments::Flag>& getFlags(void) const {return this->_flags;};

        // ------------ Operator ---------- //
        ArgParser& operator=(const ArgParser& other) = delete;
        ArgParser& operator=(ArgParser&& other) = delete;

        // ---------- Constructor --------- //
        ArgParser(const std::string& binary = "[None]", const std::string& description = "...");
        ArgParser(const ArgParser& other) = delete;
        ArgParser(ArgParser&& other) = delete;

        // ----------- Destructor --------- //
        ~ArgParser() = default;
};

} // namespace end
#endif /* ARGPARSER_H */
