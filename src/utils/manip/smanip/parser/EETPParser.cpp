/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 25/08/2026 by @author Tsukini

File Name:
##  @file EETPParser.cpp

File Description:
##  Definition of the EETP parser methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/manip/smanip/parser/EETPParser.hpp"
#include "utils/manip/iomanip/Char.hpp"
#include <cstddef>
#include <vector>
#include <string>

// payload = <tag> ETB <content>
// <tag>     (codec) -> <iv> (12 bytes) <AES-GCM tag> (16 bytes), empty when the content isn't AES encrypted
// <content> (codec) -> <type> <data>
//  - AES  : AES(<type> *(ETB <data>))     -> default payloads, a new random <iv> for each payload (GCM nonce never reused)
//  - SYN  : <type> RSA_common(ETB <data>) -> <data> = client RSA public key
//  - SO   : <type> RSA_client(ETB <data>) -> <data> = AES key
//  - EM   : <type>                        -> never encrypted, no data
// Each <data> is preceded by ETB (an empty <data> is kept) & encoded with the codec (ETB can never appear inside a <data>)

/* Special Type
 * SYN -> generate local RSA + send the public key encrypted with the common RSA
 * SO  -> generate & store AES + send it encrypted with the RSA public key of the client
 * EM  -> not encrypted (disconnection)
 * ACK -> only one <data> or less
 * NAK -> only one <data> or less
 * other -> <type> *(ETB <data>)
*/

_hot _nodiscard std::string utils::smanip::parser::EETPParser::format(std::string id, utils::smanip::parser::EETPContent content)
{
    if (content.type.size() != this->_typeSize) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid content, the type should be " + std::to_string(this->_typeSize) + " wide");
    }

    utils::security::encryption::KeyAES& keyAES = this->_aesKeys[id];
    keyAES.tag.clear(); // Reset on each new encryption
    std::string s = content.type; // <type> always in clear (except AES), needed to know how to decrypt the rest
    std::string tag; // <iv> <tag> (AES only)

    char type = content.type.front(); // Default type are only one char wide
    switch (type) {
        case static_cast<char>(utils::iomanip::Char::SYN): // Connection
        {
            utils::security::encryption::RSAKey& rsaKey = this->_rsaKeys[id];
            rsaKey.generate(); // Generate local RSA
            s += this->_commonKey.encrypt(static_cast<char>(utils::iomanip::Char::ETB) + this->_codec->encode(rsaKey.get().pub)); // common RSA (pub)
            break;
        }

        case static_cast<char>(utils::iomanip::Char::EM): // Disconnection
            break; // Never encrypted, no data

        case static_cast<char>(utils::iomanip::Char::SO): // Key exchange
        {
            keyAES.AES = this->_aesKey.generateRandomBytes(EETP_AES_KEY_SIZE); // Generate AES
            s += this->_rsaKeys[id].encrypt(static_cast<char>(utils::iomanip::Char::ETB) + this->_codec->encode(keyAES.AES)); // RSA of the client (pub)
            break;
        }

        default: // Default payload: AES encrypted
        {
            if (keyAES.AES.empty()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "No AES key for this id, the key exchange (SYN/SO) must be done first: " + id);
            }
            for (const std::string& data: content.data) {
                s += static_cast<char>(utils::iomanip::Char::ETB);
                s += this->_codec->encode(data);
            }
            keyAES.iv = this->_aesKey.generateRandomBytes(EETP_AES_IV_SIZE); // new nonce for each payload
            s = this->_aesKey.encrypt(s, keyAES);
            tag = keyAES.iv + keyAES.tag;
            break;
        }
    }

    // Encapsule the string
    std::string framed;
    if (!tag.empty()) _likely {framed += this->_codec->encode(tag);}
    framed += static_cast<char>(utils::iomanip::Char::ETB);
    framed += this->_codec->encode(s);

    return framed;
}

_hot _nodiscard utils::smanip::parser::EETPContent utils::smanip::parser::EETPParser::parse(std::string id, std::string s)
{
    utils::smanip::parser::EETPContent content;
    std::size_t pos = 0;

    // Check the minimum size (ETB + type size)
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

    // Decrypt if the tag is set (<iv> <tag>)
    if (!tag.empty()) {
        if (tag.size() != EETP_AES_IV_SIZE + EETP_AES_TAG_SIZE) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, the tag should be " + std::to_string(EETP_AES_IV_SIZE + EETP_AES_TAG_SIZE) + " bytes");
        }
        utils::security::encryption::KeyAES& keyAES = this->_aesKeys[id];
        if (keyAES.AES.empty()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "No AES key for this id, the key exchange (SYN/SO) must be done first: " + id);
        }
        keyAES.iv = tag.substr(0, EETP_AES_IV_SIZE);
        keyAES.tag = tag.substr(EETP_AES_IV_SIZE);
        s = this->_aesKey.decrypt(s, keyAES);
    }

    // Extract the type
    if (s.size() < this->_typeSize) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, can't extract the type");
    }
    content.type = s.substr(0, this->_typeSize); // <type>
    s.erase(0, this->_typeSize); // <type>

    // On special payload
    /*
     * SYN -> <data> encrypted with common RSA
     * SO  -> <data> encrypted with local RSA
     * EM  -> never encrypted
     * other -> always AES encrypted
    */
    char type = content.type.front(); // Default type are only one char wide
    switch (type) {
        case static_cast<char>(utils::iomanip::Char::SYN): // Connection
            if (!tag.empty()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, the payload of type SYN should be RSA encrypted");
            }
            s = this->_commonKey.decrypt(s); // common RSA (priv)
            break;

        case static_cast<char>(utils::iomanip::Char::SO): // Key exchange
            if (!tag.empty()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, the payload of type SO should be RSA encrypted");
            }
            s = this->_rsaKeys[id].decrypt(s); // local RSA (priv)
            break;

        case static_cast<char>(utils::iomanip::Char::EM): // Disconnection
            if (!tag.empty()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, the payload of type EM (disconnection) should never be encrypted!!!");
            }
            break;

        default:
            if (tag.empty()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, the payload isn't encrypted");
            }
            break;
    }

    // Split the data: *(ETB <data>), each <data> is encoded with the codec
    std::vector<std::string>& data = content.data;
    if (!s.empty()) {
        if (s.front() != static_cast<char>(utils::iomanip::Char::ETB)) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, each <data> should be preceded by ETB");
        }
        std::size_t begin = 1;
        while (true) {
            pos = s.find(static_cast<char>(utils::iomanip::Char::ETB), begin);
            data.emplace_back(this->_codec->decode(s.substr(begin, (pos == std::string::npos) ? std::string::npos : pos - begin)));
            if (pos == std::string::npos) break;
            begin = pos + 1;
        }
    }

    // On special payload
    /*
     * SYN -> RSA (pub) from client
     * SO  -> AES from server
     * EM  -> no <data>
     * ACK -> only one <data>
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
            this->_rsaKeys[id].set(keyPair);
            break;
        }

        case static_cast<char>(utils::iomanip::Char::SO): // Key exchange
        {
            if (data.size() != 1 || data[0].size() != EETP_AES_KEY_SIZE) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, expected exactly 1 part: <type> (SO) <data> (AES key of " + std::to_string(EETP_AES_KEY_SIZE) + " bytes)");
            }
            // Extract AES from server
            this->_aesKeys[id].AES = data[0];
            break;
        }

        case static_cast<char>(utils::iomanip::Char::EM): // Disconnection
            if (data.size() != 0) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, expected exactly no part: <type> (EM)");
            }
            break;

        case static_cast<char>(utils::iomanip::Char::ACK): // OK
            if (data.size() > 1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "Invalid transmission content, expected exactly 1 part or less: <type> (ACK) [<data> (potential information)]");
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
