+++
title = "SManip"
summary = "String tools: styled text with tags, compile time strings, codecs (Base64) and the EETP parser that frames and encrypts the messages of a connection."
keywords = "smanip string format style tag FixedString codec Base64Codec encode decode parser EETPParser EETPContent SYN SO utils_string utils_codec utils_eetp utils_smanip_format"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_SManip` (from `_Manip`) | `_Tools` | `utils::smanip` | `utils/manip/smanip/` |

String tools: styled text with tags, compile time strings, codecs and parsers. The terminal sequences are in [IOManip](page:iomanip).

## format {#format}

`utils::smanip::format(s)` replaces the tags `<style|style|...>` by the ANSI sequences ([IOManip](page:iomanip)), `<>` resets the style. Unknown styles are ignored, the names are case insensitive.

Styles: `reset`, `strong`, `dark`, `italic`, `underlined`, `flashing_fast`, `flashing_slow`, `reversed`, `hide`, `bar`, `monospace`, `framed`, `encircled`, `overlined`, `exposant`, `indice`.

```cpp
std::cout << utils::smanip::format("<strong|underlined>USAGE<> ./app [options]") << std::endl;
```

## FixedString {#fixedstring}

`FixedString<N>`: a string usable as a template argument (`template<utils::smanip::FixedString name>`), with `view()` and `size()`. Used by the observers (`Observer<"Name">`).

> [!NOTE]
> `fixed_string` (renamed `FixedString` in `v3.0.0`) and `fixed_string.hpp` were removed in `v4.0.0`.

## Codec {#codec}

`ICodec` interface: `encode(s)`, `decode(s)`.

| Codec | Description |
|---|---|
| `codec::Base64Codec` | Base 64 (OpenSSL `EVP_EncodeBlock` / `EVP_DecodeBlock`), the trailing whitespace is ignored; `Codec` error on an invalid input, `OutOfBounds` over 2 GiB |

> [!IMPORTANT]
> Needs OpenSSL at the link (`-lssl -lcrypto`, given by `utils::utils` in CMake).

## Parser {#parser}

`IParser<T>` / `AParser<T>`: `format(content)` / `parse(s)`, and `format(id, content)` / `parse(id, s)` for the parsers that keep a state per peer.

### EETP protocol {#eetp}

`parser::EETPParser` frames and encrypts the messages of a client / server connection (the id is the peer). A message is an `EETPContent{type, data}` (`type` of `typeSize` characters, default 1). The exchange is drawn in [Communication](page:network-ipc#eetp).

| Type (`Char`) | Role |
|---|---|
| `SYN` | Client → server: the client RSA public key, encrypted with the **common** RSA key |
| `SO` | Server → client: a new AES-256 key, encrypted with the client public key |
| `EM` | Disconnection, never encrypted, no data |
| `ACK` / `NAK` | At most one data |
| any other | Data encrypted with AES-256-GCM, a new random iv for each message |

Frame: `<tag> ETB <content>`, the tag being `<iv><GCM tag>` (empty for the RSA / clear types), each part encoded with the codec (default `Base64Codec`, `setCodec`).

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
> Both sides must use `v3.0.0` or later (framing changed). A tampered message fails the GCM authentication (`Decryption`, or `Codec` when the encoding is broken), a malformed frame throws `Parser`.

## In C {#in-c}

| C++ | C |
|---|---|
| `std::string` (bytes, can hold `'\0'`) | `utils_string_t*`: `utils_string_create(data, size)`, `utils_string_data`, `utils_string_size`, `utils_string_copy_r` |
| `format(s)` | `utils_smanip_format(str)` (thread local result) / `utils_smanip_format_r(str, buf, size)` |
| `Base64Codec` | `utils_codec_create_base64()`; a codec of the C side: `utils_codec_create(encode, decode, data)`; `utils_codec_encode` / `utils_codec_decode` into a `utils_string_t*` |
| `EETPContent` | `utils_eetp_content_t*` (`utils_eetp_content_create(type)`, `_add_data`, `_get_data`) |
| `EETPParser` | `utils_eetp_parser_t*`: `utils_eetp_parser_format(parser, id, content, out)` / `utils_eetp_parser_parse(parser, id, in, size, out)` |

A codec given to a parser (`utils_eetp_parser_create_with`, `utils_eetp_parser_set_codec`) belongs to the parser on success: never destroy or use it after.

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
