+++
title = "Licenses"
summary = "libutils is under MIT; the synced algorithms are under CC BY-NC-SA 4.0 and OpenSSL under Apache-2.0."
section = "resources"
order = 30
keywords = "license licenses mit cc by-nc-sa apache openssl third party notices commercial algorithms c2dmp sos"
+++

The MIT license of libutils only covers its own code: each dependency, included or linked, keeps its own license.

## Components {#components}

| Component | License | How it is used |
|---|---|---|
| libutils (its own code) | MIT ([LICENSE](https://github.com/TsukiNi22/libutils/blob/main/LICENSE)) | - |
| `include/utils/algorithms/` ([c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm), [s.o.s](https://github.com/TsukiNi22/s.o.s)) | CC BY-NC-SA 4.0 ([LICENSE.md](https://github.com/TsukiNi22/libutils/blob/main/include/utils/algorithms/LICENSE.md)) | synced from their repositories |
| [OpenSSL](https://www.openssl.org) | Apache-2.0 | linked (system package) |

> [!CAUTION]
> The synced algorithms can't be used commercially without an agreement with the author. The C API of the algorithms (`utils/algorithms/CApi.h`) is code of libutils, but it calls them: the same restriction applies to a program that uses it.

The full list is in [THIRD_PARTY_NOTICES.md](https://github.com/TsukiNi22/libutils/blob/main/THIRD_PARTY_NOTICES.md); the packages install the licenses in `share/licenses/libutils/`.
