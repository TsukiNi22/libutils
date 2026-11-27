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
##  C API of utils::security (AES & RSA keys, observers of the C side)
\**************************************************************/

#include "utils/security/encryption/CommonRSAKey.hpp"
#include "utils/security/observer/Instances.hpp"
#include "utils/security/observer/INotifier.hpp"
#include "utils/security/encryption/AESKey.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/manip/smanip/CApi.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/security/CApi.hpp"
#include "utils/security/CApi.h"
#include <string_view>
#include <cstddef>
#include <memory>
#include <string>

//----------------------------------------------------------------//
/* CHECK */

static_assert(std::string_view(C_API_SECURITY_DEFAULT_COMMON_RSA_PATH) == DEFAULT_COMMON_RSA_PATH);

//----------------------------------------------------------------//
/* TOOLS */

// Bytes of a field of the AES data
_cold _nodiscard static const char* field(const std::string& value, std::size_t* size)
{
    if (size) *size = value.size();
    return value.c_str();
}

// Link / unlink of an observer of the C side (like utils::security::observer::AObserver)
_cold static void link(utils_observer_t& observer)
{
    utils::security::observer::instances::id_handler().allocate(observer.id, observer.safe);
    for (std::unique_ptr<utils::security::observer::INotifier>& notifier: utils::security::observer::instances::notifiers())
        notifier->link(observer.id, observer.instance, observer.safe);
}

_cold static void unlink(utils_observer_t& observer)
{
    if (observer.id == 0) return;
    for (std::unique_ptr<utils::security::observer::INotifier>& notifier: utils::security::observer::instances::notifiers())
        notifier->unlink(observer.id, observer.safe);
    utils::security::observer::instances::id_handler().free(observer.id, observer.safe);
    observer.id = 0;
}

//----------------------------------------------------------------//
/* AES KEY */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_aes_key_t* utils_aes_key_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_aes_key_t* {return new utils_aes_key_t();});
}

_cold _nothrow void utils_aes_key_destroy(utils_aes_key_t* key)
{
    (void)utils::exception::c_api::guard([&](void) {delete key;}); // like free(NULL)
}

_cold _nothrow int utils_aes_key_random_bytes(const utils_aes_key_t* key, uint16_t size, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = utils::exception::c_api::not_null_ref(key, "key").generateRandomBytes(size);
    });
}

_hot _nothrow int utils_aes_key_encrypt(const utils_aes_key_t* key, const char* in, size_t size, utils_aes_data_t* data, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        const utils::security::encryption::AESKey& self = utils::exception::c_api::not_null_ref(key, "key");
        utils_aes_data_t& keys = utils::exception::c_api::not_null_ref(data, "data");
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = self.encrypt(utils::smanip::c_api::bytes(in, size, "bytes"), keys.data);
    });
}

_hot _nothrow int utils_aes_key_decrypt(const utils_aes_key_t* key, const char* in, size_t size, utils_aes_data_t* data, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        const utils::security::encryption::AESKey& self = utils::exception::c_api::not_null_ref(key, "key");
        utils_aes_data_t& keys = utils::exception::c_api::not_null_ref(data, "data");
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = self.decrypt(utils::smanip::c_api::bytes(in, size, "bytes"), keys.data);
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_aes_key_has_generate_overload(const utils_aes_key_t* key)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(key, "key").hasGenerateOverload();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_aes_key_has_set_overload(const utils_aes_key_t* key)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(key, "key").hasSetOverload();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_aes_key_has_get_overload(const utils_aes_key_t* key)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(key, "key").hasGetOverload();});
}

//----------------------------------------------------------------//
/* AES DATA */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_aes_data_t* utils_aes_data_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_aes_data_t* {return new utils_aes_data_t();});
}

_cold _nothrow void utils_aes_data_destroy(utils_aes_data_t* data)
{
    (void)utils::exception::c_api::guard([&](void) {delete data;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_aes_data_set_key(utils_aes_data_t* data, const char* bytes_, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(data, "data").data.AES = utils::smanip::c_api::bytes(bytes_, size, "bytes");});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_aes_data_set_iv(utils_aes_data_t* data, const char* bytes_, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(data, "data").data.iv = utils::smanip::c_api::bytes(bytes_, size, "bytes");});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_aes_data_set_tag(utils_aes_data_t* data, const char* bytes_, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(data, "data").data.tag = utils::smanip::c_api::bytes(bytes_, size, "bytes");});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_aes_data_get_key(const utils_aes_data_t* data, size_t* size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return field(utils::exception::c_api::not_null_ref(data, "data").data.AES, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_aes_data_get_iv(const utils_aes_data_t* data, size_t* size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return field(utils::exception::c_api::not_null_ref(data, "data").data.iv, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_aes_data_get_tag(const utils_aes_data_t* data, size_t* size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return field(utils::exception::c_api::not_null_ref(data, "data").data.tag, size);});
}

//----------------------------------------------------------------//
/* RSA KEY */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_rsa_key_t* utils_rsa_key_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_rsa_key_t* {return new utils_rsa_key_t();});
}

_cold _nothrow utils_rsa_key_t* utils_rsa_key_create_common(const char* path)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_rsa_key_t* {
        utils_rsa_key_t* key = new utils_rsa_key_t();

        try {
            key->loadCommon(path ? path : DEFAULT_COMMON_RSA_PATH);
        } catch (...) {
            delete key;
            throw;
        }
        return key;
    });
}

_cold _nothrow void utils_rsa_key_destroy(utils_rsa_key_t* key)
{
    (void)utils::exception::c_api::guard([&](void) {delete key;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_rsa_key_load_common(utils_rsa_key_t* key, const char* path)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(key, "key").loadCommon(path ? path : DEFAULT_COMMON_RSA_PATH);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_rsa_key_generate(utils_rsa_key_t* key)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(key, "key").generate();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_rsa_key_set(utils_rsa_key_t* key, const char* private_key, const char* public_key)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(key, "key").set({(private_key ? private_key : ""), (public_key ? public_key : "")});});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_rsa_key_get_private(const utils_rsa_key_t* key)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return utils::exception::c_api::not_null_ref(key, "key").get().priv.c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_rsa_key_get_public(const utils_rsa_key_t* key)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return utils::exception::c_api::not_null_ref(key, "key").get().pub.c_str();});
}

_cold _nothrow int utils_rsa_key_random_bytes(const utils_rsa_key_t* key, uint16_t size, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = utils::exception::c_api::not_null_ref(key, "key").generateRandomBytes(size);
    });
}

_hot _nothrow int utils_rsa_key_encrypt(const utils_rsa_key_t* key, const char* in, size_t size, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = utils::exception::c_api::not_null_ref(key, "key").encrypt(utils::smanip::c_api::bytes(in, size, "bytes"));
    });
}

_hot _nothrow int utils_rsa_key_decrypt(const utils_rsa_key_t* key, const char* in, size_t size, utils_string_t* out)
{
    return utils::exception::c_api::guard([&](void) {
        utils_string_t& res = utils::exception::c_api::not_null_ref(out, "output");

        res.str = utils::exception::c_api::not_null_ref(key, "key").decrypt(utils::smanip::c_api::bytes(in, size, "bytes"));
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_rsa_key_has_generate_overload(const utils_rsa_key_t* key)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(key, "key").hasGenerateOverload();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_rsa_key_has_set_overload(const utils_rsa_key_t* key)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(key, "key").hasSetOverload();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_rsa_key_has_get_overload(const utils_rsa_key_t* key)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(key, "key").hasGetOverload();});
}

//----------------------------------------------------------------//
/* OBSERVER */

_cold _nothrow utils_observer_t* utils_observer_create(const char* instance, bool safe_mode)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_observer_t* {
        const char* name = utils::exception::c_api::not_null(instance, "instance"); // before the new: a NULL instance leaked the handle
        utils_observer_t* observer = new utils_observer_t();

        observer->instance = name;
        observer->safe = safe_mode;
        try {
            link(*observer);
        } catch (...) {
            delete observer;
            throw;
        }
        return observer;
    });
}

_cold _nothrow void utils_observer_destroy(utils_observer_t* observer)
{
    (void)utils::exception::c_api::guard([&](void) {
        if (!observer) return; // like free(NULL)
        unlink(*observer);
        delete observer;
    });
}

_cold _nothrow int utils_observer_trigger(void)
{
    return utils::exception::c_api::guard([](void) {
        for (std::unique_ptr<utils::security::observer::INotifier>& notifier: utils::security::observer::instances::notifiers())
            notifier->trigger();
    });
}
