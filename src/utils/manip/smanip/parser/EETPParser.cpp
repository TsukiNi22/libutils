/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 02/08/2026 by @author Tsukini

File Name:
##  @file EETPParser.hpp

File Description:
##  Definition of the 2etp parser methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/manip/smanip/parser/EETPParser.hpp"
#include <string>

// payload = <tag> ETB <type> [<data> *(<data> ETB)]
/* Special Type
 * SYN -> generate local RSA + encrypt with common RSA
 * SO -> store AES + decrypt using local RSA + encrypt with local RSA
 * EM -> not encrypted
 * ACK -> no <data>
 * NAK -> only one <data>
 * other -> <type> [<data> *(<data> ETB)]
*/

_hot _nodiscard std::string utils::smanip::parser::EETPParser::format(std::string id, utils::smanip::parser::EETPContent content)
{
    // Try to extract the tag
    std::string tag;
}

_hot _nodiscard utils::smanip::parser::EETPContent utils::smanip::parser::EETPParser::parse(std::string id, std::string s)
{
}
