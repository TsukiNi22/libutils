/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.cpp

File Description:
##  C API of utils::smanip (byte strings, format, codecs, EETP parser)
\**************************************************************/

#include "utils/manip/smanip/codec/Base64Codec.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/manip/smanip/parser/EETPParser.hpp"
#include "utils/manip/smanip/codec/ICodec.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/manip/smanip/format.hpp"
#include "utils/manip/smanip/CApi.hpp"
#include "utils/manip/smanip/CApi.h"
#include "utils/exception/CApi.hpp"
#include <cstddef>
#include <utility>
#include <memory>
#include <string>

//----------------------------------------------------------------//
/* CLASS */

// Codec of the C side (callbacks), the error of a callback is thrown with its info
class CCodec final: public utils::smanip::codec::ICodec {
    private:
        utils_codec_fn_t _encode = nullptr;
        utils_codec_fn_t _decode = nullptr;
        void* _data = nullptr;

        // ------------ Function ---------- //
        _cold _nodiscard std::string call_(utils_codec_fn_t fn, const std::string& s) const
        {
            utils_string_t out;

            utils::exception::c_api::reset_status(); // only the error of this callback
            utils::exception::c_api::check_callback(fn(s.data(), s.size(), &out, this->_data));
            return std::move(out.str);
        };

    public:
        // ------------ Function ---------- //
        _cold _nodiscard std::string encode(std::string s) const override {return this->call_(this->_encode, s);};
        _cold _nodiscard std::string decode(std::string s) const override {return this->call_(this->_decode, s);};

        // ---------- Constructor --------- //
        CCodec(utils_codec_fn_t encode, utils_codec_fn_t decode, void* data): _encode{encode}, _decode{decode}, _data{data} { /* Nothing */ };
};

//----------------------------------------------------------------//
/* STRING */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_string_t* utils_string_create(const char* data, size_t size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_string_t* {return new utils_string_t{utils::smanip::c_api::bytes(data, size)};});
}

_cold void utils_string_destroy(utils_string_t* str)
{
    (void)utils::exception::c_api::guard([&](void) {delete str;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_string_set(utils_string_t* str, const char* data, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(str, "string").str = utils::smanip::c_api::bytes(data, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_string_append(utils_string_t* str, const char* data, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(str, "string").str += utils::smanip::c_api::bytes(data, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_string_clear(utils_string_t* str)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(str, "string").str.clear();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot const char* utils_string_data(const utils_string_t* str)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return utils::exception::c_api::not_null_ref(str, "string").str.c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot size_t utils_string_size(const utils_string_t* str)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(str, "string").str.size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot size_t utils_string_copy_r(const utils_string_t* str, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::exception::c_api::not_null_ref(str, "string").str, buf, size);});
}

//----------------------------------------------------------------//
/* FORMAT */

_cold const char* utils_smanip_format(const char* str)
{
    // xstyle: ignore-next CPP-THIS (false positive: _thread_local is the attribute macro, not a member)
    static _thread_local std::string formated;

    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {formated = utils::smanip::format(utils::exception::c_api::not_null(str, "string")); return formated.c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold size_t utils_smanip_format_r(const char* str, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::smanip::format(utils::exception::c_api::not_null(str, "string")), buf, size);});
}

//----------------------------------------------------------------//
/* CODEC */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_codec_t* utils_codec_create_base64(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_codec_t* {return new utils_codec_t{std::make_unique<utils::smanip::codec::Base64Codec>()};});
}

_cold utils_codec_t* utils_codec_create(utils_codec_fn_t encode, utils_codec_fn_t decode, void* data)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_codec_t* {
        (void)utils::exception::c_api::not_null(encode, "encode callback");
        (void)utils::exception::c_api::not_null(decode, "decode callback");
        return new utils_codec_t{std::make_unique<CCodec>(encode, decode, data)};
    });
}

_cold void utils_codec_destroy(utils_codec_t* codec)
{
    (void)utils::exception::c_api::guard([&](void) {delete codec;}); // like free(NULL)
}

_hot int utils_codec_encode(const utils_codec_t* codec, const char* in, size_t size, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        const utils_codec_t& self = utils::exception::c_api::not_null_ref(codec, "codec");
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = self.codec->encode(utils::smanip::c_api::bytes(in, size));
    });
}

_hot int utils_codec_decode(const utils_codec_t* codec, const char* in, size_t size, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        const utils_codec_t& self = utils::exception::c_api::not_null_ref(codec, "codec");
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = self.codec->decode(utils::smanip::c_api::bytes(in, size));
    });
}

//----------------------------------------------------------------//
/* EETP CONTENT */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_eetp_content_t* utils_eetp_content_create(const char* type)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_eetp_content_t* {return new utils_eetp_content_t{{utils::exception::c_api::not_null(type, "type"), {}}};});
}

_cold void utils_eetp_content_destroy(utils_eetp_content_t* content)
{
    (void)utils::exception::c_api::guard([&](void) {delete content;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_eetp_content_set_type(utils_eetp_content_t* content, const char* type)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(content, "content").content.type = utils::exception::c_api::not_null(type, "type");});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold const char* utils_eetp_content_get_type(const utils_eetp_content_t* content)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return utils::exception::c_api::not_null_ref(content, "content").content.type.c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_eetp_content_add_data(utils_eetp_content_t* content, const char* data, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(content, "content").content.data.push_back(utils::smanip::c_api::bytes(data, size));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_eetp_content_clear_data(utils_eetp_content_t* content)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(content, "content").content.data.clear();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold size_t utils_eetp_content_data_count(const utils_eetp_content_t* content)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(content, "content").content.data.size();});
}

_cold const char* utils_eetp_content_get_data(const utils_eetp_content_t* content, size_t i, size_t* size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {
        const std::string& data = utils::exception::c_api::not_null_ref(content, "content").content.data.at(i);

        if (size) *size = data.size();
        return data.c_str();
    });
}

//----------------------------------------------------------------//
/* EETP PARSER */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_eetp_parser_t* utils_eetp_parser_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_eetp_parser_t* {return new utils_eetp_parser_t{};});
}

_cold utils_eetp_parser_t* utils_eetp_parser_create_with(utils_codec_t* codec, size_t type_size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_eetp_parser_t* {
        utils_codec_t& given = utils::exception::c_api::not_null_ref(codec, "codec");
        utils_eetp_parser_t* parser = new utils_eetp_parser_t{};

        // Checked before taking the codec (still owned by the caller on error)
        try {
            parser->parser.setTypeSize(type_size);
        } catch (...) {
            delete parser;
            throw;
        }
        parser->parser.setCodec(std::move(given.codec));
        delete codec;
        return parser;
    });
}

_cold void utils_eetp_parser_destroy(utils_eetp_parser_t* parser)
{
    (void)utils::exception::c_api::guard([&](void) {delete parser;}); // like free(NULL)
}

_cold int utils_eetp_parser_set_codec(utils_eetp_parser_t* parser, utils_codec_t* codec)
{
    return utils::exception::c_api::guard([&](void) {
        utils_eetp_parser_t& self = utils::exception::c_api::not_null_ref(parser, "parser");
        utils_codec_t& given = utils::exception::c_api::not_null_ref(codec, "codec");

        self.parser.setCodec(std::move(given.codec));
        delete codec;
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_eetp_parser_set_type_size(utils_eetp_parser_t* parser, size_t type_size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").parser.setTypeSize(type_size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_eetp_parser_has_id_overload(const utils_eetp_parser_t* parser)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(parser, "parser").parser.hasIdOverload();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_eetp_parser_has_no_id_overload(const utils_eetp_parser_t* parser)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(parser, "parser").parser.hasNoIdOverload();});
}

_hot int utils_eetp_parser_format(utils_eetp_parser_t* parser, const char* id, const utils_eetp_content_t* content, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        utils_eetp_parser_t& self = utils::exception::c_api::not_null_ref(parser, "parser");
        const utils_eetp_content_t& data = utils::exception::c_api::not_null_ref(content, "content");
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = self.parser.format(utils::exception::c_api::not_null(id, "id"), data.content);
    });
}

_hot int utils_eetp_parser_parse(utils_eetp_parser_t* parser, const char* id, const char* in, size_t size, utils_eetp_content_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        utils_eetp_parser_t& self = utils::exception::c_api::not_null_ref(parser, "parser");
        utils_eetp_content_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.content = self.parser.parse(utils::exception::c_api::not_null(id, "id"), utils::smanip::c_api::bytes(in, size));
    });
}
