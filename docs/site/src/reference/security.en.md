+++
title = "Security"
summary = "Encryption (AES-256-GCM, RSA OAEP, the common RSA key) and the observers that track the instances and report the memory leaks at exit."
keywords = "security encryption aes rsa key encrypt decrypt iv tag CommonRSAKey observer notifier memory leak MemoryLeakNotifier utils_aes_key utils_rsa_key utils_observer"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Security` | `_Tools` | `utils::security` | `utils/security/` |
| `_Encryption` (from `_Security`) | `_Security` | `utils::security::encryption` | `utils/security/encryption/` |
| `_Observer` (from `_Security`, only a marker) | `_Security` | `utils::security::observer` | `utils/security/observer/` (include them directly) |

> [!IMPORTANT]
> The encryption needs OpenSSL at the link (`-lssl -lcrypto`, given by `utils::utils` in CMake).

## Keys {#keys}

Every key derives `IKey<T>` / `AKey<T>` (`T` = the key data):

| Method | Description |
|---|---|
| `generate()` | Generate a new key |
| `set(data)` / `get()` | Set / get the key data |
| `encrypt(s)` / `decrypt(s)` | With the stored key |
| `encrypt(s, data)` / `decrypt(s, data)` | With a key given at the call (`data` can be updated, ex: the AES tag) |
| `generateRandomBytes(size = 32)` | Random bytes (OpenSSL `RAND_bytes`) |
| `hasGenerateOverload()`, `hasSetOverload()`, `hasGetOverload()` | What the key implements |

> [!WARNING]
> A method a key doesn't implement ends with `FatalException(UndefinedCall)` (ex: `AESKey::generate()`).

## AESKey {#aeskey}

AES-256-GCM, the key is given at each call: `KeyAES{AES, iv, tag}`.

| Field | Size |
|---|---|
| `AES` | 32 bytes (`AES_KEY_SIZE`) |
| `iv` | at least 12 bytes (`AES_MIN_IV_SIZE`), **a new one for every message** |
| `tag` | 16 bytes (`AES_TAG_SIZE`), written by `encrypt`, checked by `decrypt` |

```cpp
#define _Encryption
#include <utils/utils.hpp>

utils::security::encryption::AESKey aes;
utils::security::encryption::KeyAES key{aes.generateRandomBytes(32), aes.generateRandomBytes(12), ""};
std::string cipher = aes.encrypt("secret", key);   // key.tag is set
std::string plain = aes.decrypt(cipher, key);      // "secret", Decryption error if modified
```

Wrong sizes throw `Encryption` / `Decryption`, data over 2 GiB `OutOfBounds`.

## RSAKey {#rsakey}

RSA with OAEP padding, `KeyPair{priv, pub}` in PEM. Data of any size (encrypted block by block).

```cpp
utils::security::encryption::RSAKey rsa;
rsa.generate();
std::string cipher = rsa.encrypt("hello");          // with get().pub
std::string plain = rsa.decrypt(cipher);            // with get().priv
```

## CommonRSAKey {#commonrsakey}

An `RSAKey` loaded from files shared by both sides (used by the EETP handshake, [SManip](page:smanip#eetp)): `loadCommon(path = "~/.ssh/common")` reads `<path>` (private) and `<path>.pub` (public).

| Function | Description |
|---|---|
| `key_to_string(std::vector<std::uint8_t>)` | Bytes to `std::string` (the `\0` are kept) |
| `string_to_key(std::string)` | `std::string` to bytes |

> [!NOTE]
> `keyToString` / `stringToKey` (renamed `key_to_string` / `string_to_key` in `v3.0.0`) were removed in `v4.0.0`.

## Observer {#observer}

Most classes of the library inherit `Observer<"Name">`: each instance registers an id in a global handler, and the **memory leak notifier** reports the instances still alive when the program ends.

| Class | Description |
|---|---|
| `Observer<"Name">` | Tracks the instances of a class (thread safe) |
| `UnsafeObserver<"Name">` | Same without lock |
| `AObserver<"Name", safe_mode>` / `IObserver` | Base classes |
| `INotifier` / `ANotifier` | `link(id, instance)`, `unlink(id)`, `clear()`, `trigger()` |
| `MemoryLeakNotifier` | At exit (or on `trigger()`), prints the classes still alive and their count |
| `instances::id_handler()` | Global `IdHandler<std::uint64_t>` of the observers |
| `instances::notifiers()` | Array of the notifiers (`[0]` = `MemoryLeakNotifier`) |

Inherit it **privately**: `class Server: private utils::security::observer::Observer<"Server">`. Output of a leak:

```text
[WARNING] Memory leak detected (origin: <module>)
-- At least one instance wasn't properly closed --
  2 - ISocket
  1 - Client
```

```cpp
#include <utils/utils.hpp>
#include <utils/security/observer/Observer.hpp>

class Session: private utils::security::observer::Observer<"Session"> {};

int main(void)
{
    new Session(); // never deleted: reported at exit
    return 0;
}
```

## In C {#in-c}

| C++ | C |
|---|---|
| `AESKey` + `KeyAES` | `utils_aes_key_t*` + `utils_aes_data_t*` (`utils_aes_data_set_key` / `_set_iv`, the tag set by `utils_aes_key_encrypt`) |
| `RSAKey` / `CommonRSAKey` | `utils_rsa_key_t*`: `utils_rsa_key_create()` then `utils_rsa_key_generate`, or `utils_rsa_key_create_common(path)` (`NULL` = `~/.ssh/common`) |
| `encrypt` / `decrypt` | `utils_aes_key_encrypt(key, in, size, data, out)`, `utils_rsa_key_encrypt(key, in, size, out)`... into a `utils_string_t*` ([SManip](page:smanip#in-c)) |
| `Observer<"Name">` | `utils_observer_t*`: `utils_observer_create(instance, safe_mode)`; an observer never destroyed is reported as a leak; `utils_observer_trigger()` reports now |

```c
#define _Encryption
#define _SManip
#include <utils.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    utils_aes_key_t* aes = utils_aes_key_create();
    utils_aes_data_t* data = utils_aes_data_create();
    utils_string_t* bytes = utils_string_create(NULL, 0);
    utils_string_t* cipher = utils_string_create(NULL, 0);
    utils_string_t* plain = utils_string_create(NULL, 0);
    int ret = 1;

    if (aes && data && bytes && cipher && plain
        && utils_aes_key_random_bytes(aes, 32, bytes) == UTILS_OK
        && utils_aes_data_set_key(data, utils_string_data(bytes), utils_string_size(bytes)) == UTILS_OK
        && utils_aes_key_random_bytes(aes, 12, bytes) == UTILS_OK
        && utils_aes_data_set_iv(data, utils_string_data(bytes), utils_string_size(bytes)) == UTILS_OK
        && utils_aes_key_encrypt(aes, "secret", strlen("secret"), data, cipher) == UTILS_OK  // the tag of data is set
        && utils_aes_key_decrypt(aes, utils_string_data(cipher), utils_string_size(cipher), data, plain) == UTILS_OK) {
        printf("%s\n", utils_string_data(plain)); // secret
        ret = 0;
    }
    utils_string_destroy(plain);
    utils_string_destroy(cipher);
    utils_string_destroy(bytes);
    utils_aes_data_destroy(data);
    utils_aes_key_destroy(aes);
    return ret;
}
```
