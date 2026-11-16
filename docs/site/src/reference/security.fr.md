+++
title = "Security"
summary = "Chiffrement (AES-256-GCM, RSA OAEP, la clé RSA commune) et observers qui suivent les instances et signalent les fuites mémoire à la sortie."
keywords = "sécurité chiffrement aes rsa clé chiffrer déchiffrer iv tag CommonRSAKey observer notifier fuite mémoire MemoryLeakNotifier utils_aes_key utils_rsa_key utils_observer"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_Security` | `_Tools` | `utils::security` | `utils/security/` |
| `_Encryption` (depuis `_Security`) | `_Security` | `utils::security::encryption` | `utils/security/encryption/` |
| `_Observer` (depuis `_Security`, simple marqueur) | `_Security` | `utils::security::observer` | `utils/security/observer/` (à inclure directement) |

> [!IMPORTANT]
> Le chiffrement a besoin d'OpenSSL à l'édition de liens (`-lssl -lcrypto`, fournis par `utils::utils` en CMake).

## Clés {#keys}

Chaque clé dérive de `IKey<T>` / `AKey<T>` (`T` = les données de la clé) :

| Méthode | Description |
|---|---|
| `generate()` | Génère une nouvelle clé |
| `set(data)` / `get()` | Définit / récupère les données de la clé |
| `encrypt(s)` / `decrypt(s)` | Avec la clé stockée |
| `encrypt(s, data)` / `decrypt(s, data)` | Avec une clé donnée à l'appel (`data` peut être mis à jour, ex : le tag AES) |
| `generateRandomBytes(size = 32)` | Octets aléatoires (`RAND_bytes` d'OpenSSL) |
| `hasGenerateOverload()`, `hasSetOverload()`, `hasGetOverload()` | Ce que la clé implémente |

> [!WARNING]
> Une méthode qu'une clé n'implémente pas se termine par `FatalException(UndefinedCall)` (ex : `AESKey::generate()`).

## AESKey {#aeskey}

AES-256-GCM, la clé est donnée à chaque appel : `KeyAES{AES, iv, tag}`.

| Champ | Taille |
|---|---|
| `AES` | 32 octets (`AES_KEY_SIZE`) |
| `iv` | au moins 12 octets (`AES_MIN_IV_SIZE`), **un nouveau pour chaque message** |
| `tag` | 16 octets (`AES_TAG_SIZE`), écrit par `encrypt`, vérifié par `decrypt` |

```cpp
#define _Encryption
#include <utils/utils.hpp>

utils::security::encryption::AESKey aes;
utils::security::encryption::KeyAES key{aes.generateRandomBytes(32), aes.generateRandomBytes(12), ""};
std::string cipher = aes.encrypt("secret", key);   // key.tag is set
std::string plain = aes.decrypt(cipher, key);      // "secret", Decryption error if modified
```

Des tailles incorrectes lèvent `Encryption` / `Decryption`, des données de plus de 2 Gio `OutOfBounds`.

## RSAKey {#rsakey}

RSA avec padding OAEP, `KeyPair{priv, pub}` en PEM. Données de toute taille (chiffrées bloc par bloc).

```cpp
utils::security::encryption::RSAKey rsa;
rsa.generate();
std::string cipher = rsa.encrypt("hello");          // with get().pub
std::string plain = rsa.decrypt(cipher);            // with get().priv
```

## CommonRSAKey {#commonrsakey}

Une `RSAKey` chargée depuis des fichiers partagés par les deux côtés (utilisée par le handshake EETP, [SManip](page:smanip#eetp)) : `loadCommon(path = "~/.ssh/common")` lit `<path>` (privée) et `<path>.pub` (publique).

| Fonction | Description |
|---|---|
| `key_to_string(std::vector<std::uint8_t>)` | Octets vers `std::string` (les `\0` sont conservés) |
| `string_to_key(std::string)` | `std::string` vers octets |

> [!NOTE]
> `keyToString` / `stringToKey` (renommées `key_to_string` / `string_to_key` en `v3.0.0`) ont été supprimées en `v4.0.0`.

## Observer {#observer}

La plupart des classes de la bibliothèque héritent de `Observer<"Name">` : chaque instance enregistre un id dans un handler global, et le **notifier de fuites mémoire** signale les instances encore en vie à la fin du programme.

| Classe | Description |
|---|---|
| `Observer<"Name">` | Suit les instances d'une classe (thread safe) |
| `UnsafeObserver<"Name">` | Pareil sans verrou |
| `AObserver<"Name", safe_mode>` / `IObserver` | Classes de base |
| `INotifier` / `ANotifier` | `link(id, instance)`, `unlink(id)`, `clear()`, `trigger()` |
| `MemoryLeakNotifier` | À la sortie (ou sur `trigger()`), affiche les classes encore en vie et leur nombre |
| `instances::id_handler()` | `IdHandler<std::uint64_t>` global des observers |
| `instances::notifiers()` | Tableau des notifiers (`[0]` = `MemoryLeakNotifier`) |

Héritez-en en **privé** : `class Server: private utils::security::observer::Observer<"Server">`. Sortie d'une fuite :

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

## En C {#in-c}

| C++ | C |
|---|---|
| `AESKey` + `KeyAES` | `utils_aes_key_t*` + `utils_aes_data_t*` (`utils_aes_data_set_key` / `_set_iv`, le tag défini par `utils_aes_key_encrypt`) |
| `RSAKey` / `CommonRSAKey` | `utils_rsa_key_t*` : `utils_rsa_key_create()` puis `utils_rsa_key_generate`, ou `utils_rsa_key_create_common(path)` (`NULL` = `~/.ssh/common`) |
| `encrypt` / `decrypt` | `utils_aes_key_encrypt(key, in, size, data, out)`, `utils_rsa_key_encrypt(key, in, size, out)`... dans un `utils_string_t*` ([SManip](page:smanip#in-c)) |
| `Observer<"Name">` | `utils_observer_t*` : `utils_observer_create(instance, safe_mode)` ; un observer jamais détruit est signalé comme une fuite ; `utils_observer_trigger()` fait le rapport tout de suite |

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
