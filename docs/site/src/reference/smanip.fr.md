+++
title = "SManip"
summary = "Outils de chaînes : texte stylé avec des balises, chaînes à la compilation, codecs (Base64) et le parser EETP qui encadre et chiffre les messages d'une connexion."
keywords = "smanip chaîne format style balise FixedString codec Base64Codec encoder décoder parser EETPParser EETPContent SYN SO utils_string utils_codec utils_eetp utils_smanip_format"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_SManip` (depuis `_Manip`) | `_Tools` | `utils::smanip` | `utils/manip/smanip/` |

Outils de chaînes : texte stylé avec des balises, chaînes à la compilation, codecs et parsers. Les séquences du terminal sont dans [IOManip](page:iomanip).

## format {#format}

`utils::smanip::format(s)` remplace les balises `<style|style|...>` par les séquences ANSI ([IOManip](page:iomanip)), `<>` réinitialise le style. Les styles inconnus sont ignorés, les noms ne tiennent pas compte de la casse.

Styles : `reset`, `strong`, `dark`, `italic`, `underlined`, `flashing_fast`, `flashing_slow`, `reversed`, `hide`, `bar`, `monospace`, `framed`, `encircled`, `overlined`, `exposant`, `indice`.

```cpp
std::cout << utils::smanip::format("<strong|underlined>USAGE<> ./app [options]") << std::endl;
```

## FixedString {#fixedstring}

`FixedString<N>` : une chaîne utilisable comme argument de template (`template<utils::smanip::FixedString name>`), avec `view()` et `size()`. Utilisée par les observers (`Observer<"Name">`).

> [!NOTE]
> `fixed_string` (renommé `FixedString` en `v3.0.0`) et `fixed_string.hpp` ont été supprimés en `v4.0.0`.

## Codec {#codec}

Interface `ICodec` : `encode(s)`, `decode(s)`.

| Codec | Description |
|---|---|
| `codec::Base64Codec` | Base 64 (`EVP_EncodeBlock` / `EVP_DecodeBlock` d'OpenSSL), les espaces de fin sont ignorés ; erreur `Codec` sur une entrée invalide, `OutOfBounds` au-delà de 2 Gio |

> [!IMPORTANT]
> A besoin d'OpenSSL à l'édition de liens (`-lssl -lcrypto`, fournis par `utils::utils` en CMake).

## Parser {#parser}

`IParser<T>` / `AParser<T>` : `format(content)` / `parse(s)`, et `format(id, content)` / `parse(id, s)` pour les parsers qui gardent un état par pair.

### Protocole EETP {#eetp}

`parser::EETPParser` encadre et chiffre les messages d'une connexion client / serveur (l'id est le pair). Un message est un `EETPContent{type, data}` (`type` de `typeSize` caractères, 1 par défaut). L'échange est schématisé dans [Communication](page:network-ipc#eetp).

| Type (`Char`) | Rôle |
|---|---|
| `SYN` | Client → serveur : la clé publique RSA du client, chiffrée avec la clé RSA **commune** |
| `SO` | Serveur → client : une nouvelle clé AES-256, chiffrée avec la clé publique du client |
| `EM` | Déconnexion, jamais chiffrée, pas de données |
| `ACK` / `NAK` | Au plus une donnée |
| tout autre | Données chiffrées en AES-256-GCM, un nouvel iv aléatoire pour chaque message |

Trame : `<tag> ETB <content>`, le tag étant `<iv><GCM tag>` (vide pour les types RSA / en clair), chaque partie encodée avec le codec (par défaut `Base64Codec`, `setCodec`).

```cpp
#define _SManip
#include <utils/utils.hpp>

utils::smanip::parser::EETPParser client, server; // common RSA key read from ~/.ssh/common(.pub)
std::string syn = client.format("server", {std::string(1, static_cast<char>(utils::iomanip::Char::SYN)), {}});
(void)server.parse("client", syn);
std::string so = server.format("client", {std::string(1, static_cast<char>(utils::iomanip::Char::SO)), {}});
(void)client.parse("server", so);

std::string framed = client.format("server", {"A", {"hello", "world"}});
utils::smanip::parser::EETPContent content = server.parse("client", framed); // type "A", data {"hello", "world"}
```

> [!WARNING]
> Les deux côtés doivent utiliser `v3.0.0` ou plus récent (la trame a changé). Un message altéré échoue à l'authentification GCM (`Decryption`, ou `Codec` quand l'encodage est cassé), une trame mal formée lève `Parser`.

## En C {#in-c}

| C++ | C |
|---|---|
| `std::string` (octets, peut contenir `'\0'`) | `utils_string_t*` : `utils_string_create(data, size)`, `utils_string_data`, `utils_string_size`, `utils_string_copy_r` |
| `format(s)` | `utils_smanip_format(str)` (résultat thread local) / `utils_smanip_format_r(str, buf, size)` |
| `Base64Codec` | `utils_codec_create_base64()` ; un codec côté C : `utils_codec_create(encode, decode, data)` ; `utils_codec_encode` / `utils_codec_decode` dans un `utils_string_t*` |
| `EETPContent` | `utils_eetp_content_t*` (`utils_eetp_content_create(type)`, `_add_data`, `_get_data`) |
| `EETPParser` | `utils_eetp_parser_t*` : `utils_eetp_parser_format(parser, id, content, out)` / `utils_eetp_parser_parse(parser, id, in, size, out)` |

Un codec donné à un parser (`utils_eetp_parser_create_with`, `utils_eetp_parser_set_codec`) appartient au parser en cas de succès : ne jamais le détruire ni l'utiliser ensuite.

```c
#define _SManip
#include <utils.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    utils_codec_t* codec = utils_codec_create_base64();
    utils_string_t* encoded = utils_string_create(NULL, 0);
    utils_string_t* decoded = utils_string_create(NULL, 0);
    int ret = 1;

    if (codec && encoded && decoded
        && utils_codec_encode(codec, "hello", strlen("hello"), encoded) == UTILS_OK
        && utils_codec_decode(codec, utils_string_data(encoded), utils_string_size(encoded), decoded) == UTILS_OK) {
        printf("%s -> %s\n", utils_string_data(encoded), utils_string_data(decoded)); // aGVsbG8= -> hello
        printf("%s\n", utils_smanip_format("<strong|underlined>USAGE<> ./app [options]"));
        ret = 0;
    }
    utils_string_destroy(decoded);
    utils_string_destroy(encoded);
    utils_codec_destroy(codec);
    return ret;
}
```
