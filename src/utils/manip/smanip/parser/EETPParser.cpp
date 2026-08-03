/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 03/08/2026 by @author Tsukini

File Name:
##  @file EETPParser.hpp

File Description:
##  Definition of the 2etp parser methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/manip/smanip/parser/EETPParser.hpp"
#include "utils/manip/iomanip/Char.hpp"
#include <cstddef>
#include <string>

// payload = <tag> ETB <type> [<data> *(<data> ETB)]
// <tag> (base64)
// ETB
// <payload> (base64) -> <type> (AES only) [<data> *(<data> ETB)] (AES | RSA)

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
    std::string s;
    bool spe = false;

    // Dispatch for special encryption
    /*
     * SYN -> encrypted with common RSA
     * SO  -> encrypted with local RSA
     * EM  -> never encrypted
    */
    if (content.type.size() != this->_typeSize) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid content, the type should be " + std::to_string(this->_typeSize) + " wide");
    }
    char type = content.type.front(); // Default type are only one char wide
    switch (type) {
        case static_cast<char>(utils::iomanip::Char::SYN): // Connection
        {
            utils::security::encryption::RSAKey& RSAKey = this->_RSAKeys[id];
            RSAKey.generate(); // Generate local RSA
            s += content.type;
            s += RSAKey.get().pub; // local RSA (pub)
            s = this->_commonKey.encrypt(s); // common RSA (pub)
            spe = true; break;
        }

        case static_cast<char>(utils::iomanip::Char::EM): // Disconnection
            s += content.type; // Never encrypted
            spe = true; break;

        case static_cast<char>(utils::iomanip::Char::SO): // Key exchange
            // Generate AES
            utils::security::encryption::KeyAES& keyAES = this->_KeysAES[id];
            keyAES.AES = this->_AESKey.generateRandomBytes(32);
            keyAES.iv = this->_AESKey.generateRandomBytes(16);
            s += content.type;
            s += keyAES.AES;
            s += static_cast<char>(utils::iomanip::Char::ETB);
            s += keyAES.iv;
            s = this->_RSAKeys[id].encrypt(s); // local RSA (pub)
            spe = true; break;
    }

    // On not special encryption payload
    utils::security::encryption::KeyAES& keyAES = this->_KeysAES[id];
    keyAES.tag.clear(); // Reset on each new encryption
    if (!spe) _likely {
        // Build content
        s += content.type;
        for (std::size_t i = 0; i < content.data.size(); ++i) {
            if (i != 0) _likely {s += static_cast<char>(utils::iomanip::Char::ETB);}
            s += content.data[i];
        }

        // Encrypt
        s = this->_AESKey.encrypt(s, keyAES);
    }

    // Encapsule the string
    std::string framed;
    framed += static_cast<char>(utils::iomanip::Char::ENQ);
    if (!keyAES.tag.empty()) _likely {framed += this->_codec->encode(keyAES.tag);}
    framed += static_cast<char>(utils::iomanip::Char::ETB);
    framed += this->_codec->encode(s);
    framed += static_cast<char>(utils::iomanip::Char::EOT);

    return framed;
}

_hot _nodiscard utils::smanip::parser::EETPContent utils::smanip::parser::EETPParser::parse(std::string id, std::string s)
{
    utils::smanip::parser::EETPContent content;
    std::size_t pos = 0;

    // Check the minum size (ETB + type size)
    if (s.size() < 1 + this->_typeSize) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "The transmission content is too small, at least " + std::to_string(1 + this->_typeSize) + " bytes");
    }

    // Extract the tag
    std::string tag;
    pos = s.find(static_cast<char>(utils::iomanip::Char::ETB));
    if (pos == std::string::npos) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, can't extract tag");
    }
    tag = s.substr(0, pos); // <tag>
    s.erase(0, pos + 1); // <tag> + ETB
    if (s.empty()) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "The transmission content is too small, no more bytes after: <tag> ETB");
    }

    // Decode the content (default: base64)
    if (!tag.empty()) _likely {tag = this->_codec->decode(tag);}
    s = this->_codec->decode(s);

    // Decrypt if the tag is set
    if (!tag.empty()) {
        utils::security::encryption::KeyAES& keyAES = this->_KeysAES[id];
        keyAES.tag = tag;
        s = this->_AESKey.decrypt(s, keyAES);
    }

    // Extract the type
    content.type = s.substr(0, this->_typeSize); // <type>
    s.erase(0, this->_typeSize); // <type>

    // On special payload
    /*
     * SYN -> encrypted with common RSA
     * SO  -> encrypted with local RSA
    */
    char type = content.type.front(); // Default type are only one char wide
    switch (type) {
        case static_cast<char>(utils::iomanip::Char::SYN): // Connection
            s = this->_commonKey.decrypt(s); // common RSA (priv)
            break;

        case static_cast<char>(utils::iomanip::Char::SO): // Key exchange
            s = this->_RSAKeys[id].decrypt(s); // local RSA (priv)
            break;
    }

    // Split the lasting char on ETB
    std::vector<std::string>& data = content.data;
    std::size_t begin = 0;
    while (true) {
        pos = s.find(static_cast<char>(utils::iomanip::Char::ETB), begin);
        if (pos == std::string::npos) {
            data.emplace_back(s.substr(begin));
            break;
        }
        data.emplace_back(s.substr(begin, pos - begin));
        begin = pos + 1;
    }

    // On special payload
    /*
     * SYN -> RSA (pub) from client
     * SO  -> AES from server
     * EM  -> no <data>
     * ACK -> no <data>
     * NAK -> only one <data>
    */
    switch (type) {
        case static_cast<char>(utils::iomanip::Char::SYN): // Connection
        {
            if (data.size() != 1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, expected exactly 1 part: <type> (SYN) <data> (RSA public key)");
            }
            // Extract pub from client
            utils::security::encryption::KeyPair keyPair;
            keyPair.pub = data[0];
            this->_RSAKeys[id].set(keyPair);
            break;
        }

        case static_cast<char>(utils::iomanip::Char::SO): // Key exchange
        {
            if (data.size() != 2) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, expected exactly 1 part: <type> (SO) <data> (AES key) ETB <data> (AES iv)");
            }
            // Extract AES from server
            utils::security::encryption::KeyAES& keyAES = this->_KeysAES[id];
            keyAES.AES = data[0];
            keyAES.iv = data[1];
            break;
        }

        case static_cast<char>(utils::iomanip::Char::EM): // Disconnection
            if (!tag.empty()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, the payload of type EM (disconnection) should never be encrypted!!!");
            } else if (data.size() != 0) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, expected exactly no part: <type> (EM)");
            }
            break;

        case static_cast<char>(utils::iomanip::Char::ACK): // OK
            if (data.size() != 0) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, expected exactly no part: <type> (ACK)");
            }
            break;

        case static_cast<char>(utils::iomanip::Char::NAK): // KO
            if (data.size() > 1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, expected exactly 1 part or less: <type> (NAK) [<data> (potential information)]");
            }
            break;
    }

    return content;
}
