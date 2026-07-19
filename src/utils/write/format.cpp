/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/07/2026 by @author Tsukini

File Name:
##  @file format.cpp

File Description:
##  Format a string for the utils::write 
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/write/ANSI.hpp"
#include "utils/write/format.hpp"
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cstddef>
#include <string>

// Map of ANSI sequence avaible for this format function
static const std::unordered_map<std::string, std::function<void(std::string&)>> ansi = {
    {"strong",          [](std::string& s){s += utils::write::strong();}},
    {"dark",            [](std::string& s){s += utils::write::dark();}},
    {"italic",          [](std::string& s){s += utils::write::italic();}},
    {"underlined",      [](std::string& s){s += utils::write::underlined();}},
    {"flashing_fast",   [](std::string& s){s += utils::write::flashing_fast();}},
    {"flashing_slow",   [](std::string& s){s += utils::write::flashing_slow();}},
    {"reversed",        [](std::string& s){s += utils::write::reversed();}},
    {"hide",            [](std::string& s){s += utils::write::hide();}},
    {"bar",             [](std::string& s){s += utils::write::bar();}},
    {"monospace",       [](std::string& s){s += utils::write::monospace();}},
    {"framed",          [](std::string& s){s += utils::write::framed();}},
    {"encircled",       [](std::string& s){s += utils::write::encircled();}},
    {"overlined",       [](std::string& s){s += utils::write::overlined();}},
    {"exposant",        [](std::string& s){s += utils::write::exposant();}},
    {"indice",          [](std::string& s){s += utils::write::indice();}},
};

nodiscard std::string utils::write::format(const std::string& s)
{
    // Init the string
    std::string formated;
    formated.clear();

    // Format the string
    for (std::size_t i = 0; i < s.size();) {

        // Detect the escaped sequence
        if (s[i] == '\\' && i + 1 < s.size() && (s[i + 1] == '<' || s[i + 1] == '>')) {
            formated += s[i + 1];
            i += 2;
        }

        // Detect the start of sequence
        else if (s[i] == '<') {

            // Search for the end of the sequence, ignore "\>"
            std::size_t end = s.find('>', i + 1);
            if (end == std::string::npos) { // No end found
                formated += s[i++];
                continue;
            }
            std::string sequence = s.substr(i + 1, end - i - 1);

            // Do the sequence for each part of the sequence separated by '|'
            std::size_t start = 0;
            while (start < sequence.size()) {
                // Search for the delimitor inside the sequence to cut the content on '|'
                std::size_t sep = sequence.find('|', start);
                std::string token = sequence.substr(start, (sep == std::string::npos) ? std::string::npos : (sep - start));

                // Try to do the token found
                // auto -> std::unordered_map<std::string, std::function<void(std::string&)>>::iterator
                std::transform(token.begin(), token.end(), token.begin(), [](unsigned char c){ return std::tolower(c); });
                auto it = ansi.find(token);
                if (it != ansi.end())
                    it->second(formated);

                // Detect the end of the sequence
                if (sep == std::string::npos) {
                    start = sep;
                    break;
                }

                // inc
                start = sep + 1;
            }

            // Detect <> -> reset sequence
            if (start == 0)
                formated += utils::write::reset();

            // inc
            i = end + 1;
        }

        // Add char for those out of sequence
        else
            formated += s[i++];
    }

    return formated;
}
