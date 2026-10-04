# Third-party notices

The MIT license of `libutils` ([LICENSE](LICENSE)) only covers the code of `libutils` itself.
The components below keep their own license.

| Component | Version | License | How it is used | Obligation |
| --------- | ------- | ------- | -------------- | ---------- |
| [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) | synced (`include/utils/algorithms/c2dmp-hsm/`) | CC BY-NC-SA 4.0 | header-only, shipped in the `libutils-dev` packages | attribution, **no commercial use**, adaptations under the same license |
| [s.o.s](https://github.com/TsukiNi22/s.o.s) | synced (`include/utils/algorithms/sos/`) | CC BY-NC-SA 4.0 | header-only, shipped in the `libutils-dev` packages | attribution, **no commercial use**, adaptations under the same license |
| [OpenSSL](https://www.openssl.org) | >= 3.0 (system package) | Apache-2.0 | dynamically linked (`libssl`, `libcrypto`), not redistributed | credit + license text when OpenSSL is shipped with a product |
| [GoogleTest](https://github.com/google/googletest) | system package | BSD-3-Clause | unit tests only, not distributed | none |

## c2dmp-hsm and s.o.s

Copyright (c) 2026 Tsukini.
Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
Full text: [include/utils/algorithms/LICENSE.md](include/utils/algorithms/LICENSE.md), <https://creativecommons.org/licenses/by-nc-sa/4.0/>.
Commercial use requires a separate agreement with the author (<https://github.com/TsukiNi22>).

## OpenSSL

Copyright (c) 1998-2025 The OpenSSL Project Authors.
Copyright (c) 1995-1998 Eric A. Young, Tim J. Hudson.
Licensed under the Apache License 2.0: <https://www.openssl.org/source/license.html>
(installed with the system package, ex: `/usr/share/licenses/openssl-libs/LICENSE.txt`).
