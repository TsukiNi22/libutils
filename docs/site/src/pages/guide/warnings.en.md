+++
title = "Warnings"
summary = "Enable or disable the compile time warnings of libutils, all together, by domain or one by one."
section = "guide"
order = 40
keywords = "warning _Warning _NoWarning DEPRECATED_WARNING BACKWARD_COMPATIBILITY_WARNING LINKER_WARNING USAGE_WARNING NO_ migration deprecated"
+++

By default every warning of the library is disabled, except the forced ones (a missing header file, a fatal one). They are chosen with defines, before the include or for the whole project (`add_definitions(-D_NoWarning)`).

## All {#all}

| Define | Effect |
|---|---|
| `_Warning` | Every warning |
| `_NoWarning` | No warning |

Every define below has a disable version, `NO_<define>`. The names follow one pattern:

```text
domain warning (group):        <domain_name>_WARNING
esoteric warning (individual): <thing_name>_<domain_name>_WARNING
warning disable:               NO_<warning>
```

## Domains {#domains}

| Name | Define | Warns on |
|---|---|---|
| Deprecated | `DEPRECATED_WARNING` | Use of something deprecated (only with the attribute `_deprecated(info)`) |
| Backward compatibility | `BACKWARD_COMPATIBILITY_WARNING` | Use of a renamed / old API kept for compatibility (`_migration(x, y, z)`, `_legacy`) |
| Linker | `LINKER_WARNING` | Dependencies needed at the link, such as `-lssl -lcrypto` for the encryption |
| Usage | `USAGE_WARNING` | Things to use carefully (promises needed for custom things...) |

## One by one {#individual}

| Name | Domain | Define |
|---|---|---|
| Base64 Codec | Linker | `BASE64CODEC_LINKER_WARNING` |
| RSA Key | Linker | `RSAKEY_LINKER_WARNING` |
| AES Key | Linker | `AESKEY_LINKER_WARNING` |
| Fatal Exception | Usage | `FATALEXCEPTION_USAGE_WARNING` |
| EETP Parser | Usage | `EETPPARSER_USAGE_WARNING` |

## Example {#example}

```cpp
#define _Warning                            // everything
#define NO_BACKWARD_COMPATIBILITY_WARNING   // except the migration warnings
#include <utils/utils.hpp>
```

In C, `<utils.h>` knows `_Warning` / `_NoWarning` for the deprecation warnings (`DEPRECATED_WARNING`, `BACKWARD_COMPATIBILITY_WARNING`).
