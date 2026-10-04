# libutils: dependencies audit

**Date**: October 4, 2026 · **Sources**: `CMakeLists.txt`, `tests/CMakeLists.txt`, `cmake/package/utilsConfig.cmake.in`, `include/`, `src/`, `README.md`, `LICENSE`, `THIRD_PARTY_NOTICES.md`, `include/utils/algorithms/LICENSE.md`, rpm database, `dnf updateinfo`, GitHub API

> **How to read this document**
>
> 1. Summary.
> 2. Method: tools and limits.
> 3. Licenses: one row per dependency.
> 4. Obligations: what is credited and where.
> 5. Vulnerabilities.
>
> This is a technical review, **not legal advice**.

## Contents

1. Summary
2. Method
3. Licenses
4. Obligations
5. Vulnerabilities
6. Sources

## 1. Summary

| Point | Conclusion |
|---|---|
| **License scope** | the MIT license only covers libutils' own code: stated in the README, `THIRD_PARTY_NOTICES.md` and `include/utils/algorithms/LICENSE.md` |
| **Synced algorithms** | `c2dmp-hsm` and `s.o.s` are **CC BY-NC-SA 4.0** (no commercial use, share-alike), full text in `include/utils/algorithms/LICENSE.md`, RPM license `MIT AND CC-BY-NC-SA-4.0` |
| **Vulnerabilities** | **0** advisory (OpenSSL **3.5.9-1.fc44**) |
| **Packaging** | `libutils-dev` requires `openssl-devel` (RPM) / `libssl-dev` (DEB), the public headers include OpenSSL |
| **Third-party licenses** | **9** dependencies: permissive **2**, permissive-notice **4**, system-runtime **3**; no copyleft, no unknown license |

**Low**: the synced headers carry no SPDX line; the upstream sync workflows (`c2dmp-hsm`, `s.o.s`) run `rm -rf include/utils/algorithms/<algo>` before copying, so the line must be added upstream (the license file is one level above the synced folders for the same reason).

## 2. Method

| Item | Value |
|---|---|
| Working tree | commit `2b46e19` + the working tree changes |
| Inventory | `deps.py --transitive` |
| Vulnerabilities | `vulns.py` (OSV, `dnf updateinfo`, GitHub advisories), recent window 90 days |
| Manual | `LICENSE`, `README.md`, `THIRD_PARTY_NOTICES.md`, CPack fields, upstream sync workflow (`gh api .../sync.yml`) |
| Limits | Fedora only (the Debian `libssl3` path wasn't checked); `libutils-dev-pre` 2.13.22 reported by the script is libutils itself (installed), not a dependency |

## 3. Licenses

| Dependency | Version | License | Category | Scope | Obligation | Where it is stated |
|---|---|---|---|---|---|---|
| OpenSSL (`openssl-libs`, `openssl-devel`) | 3.5.9-1.fc44 | Apache-2.0 | permissive-notice | product (dynamic) | credit when shipped | `THIRD_PARTY_NOTICES.md` |
| c2dmp-hsm (synced) | `d6fedbb` | CC BY-NC-SA 4.0 | non-commercial | product (header-only) | attribution, no commercial use, share-alike | `include/utils/algorithms/LICENSE.md`, README, notices, RPM license |
| s.o.s (synced) | `e7d2eb5` | CC BY-NC-SA 4.0 | non-commercial | product (header-only) | same | same |
| GoogleTest | 1.17.0-2.fc44 | BSD-3-Clause and Apache-2.0 | permissive-notice | test | none | notices |
| zlib-ng-compat, glibc, libgcc, libstdc++ | system | Zlib / LGPL / GPL + runtime exception | permissive / system-runtime | — | none | — |

Linking: libutils is a **static** library linking `OpenSSL::SSL` / `OpenSSL::Crypto` `PRIVATE`, re-exported by `find_dependency(OpenSSL REQUIRED)`; OpenSSL stays a dynamic system library. The synced algorithms are header-only: their code is compiled into every binary that includes them.

## 4. Obligations

| What | Where | Status |
|---|---|---|
| libutils MIT license, limited to libutils' own code | `LICENSE`, README caution | ok |
| CC BY-NC-SA 4.0 of the algorithms | `include/utils/algorithms/LICENSE.md`, installed as `share/licenses/<package>/LICENSE-algorithms.md` | ok |
| OpenSSL / GoogleTest credits | `THIRD_PARTY_NOTICES.md`, installed next to `LICENSE` | ok |
| RPM license field | `MIT AND CC-BY-NC-SA-4.0` | ok |
| SPDX line in the synced headers | upstream sync workflows | **to do upstream** |

## 5. Vulnerabilities

No advisory for the installed versions (OSV: nothing pinned; dnf: 9 packages checked; GitHub: 3 repositories).

## 6. Sources

**Project**: `LICENSE`, `README.md:12-23`, `THIRD_PARTY_NOTICES.md`, `include/utils/algorithms/LICENSE.md`, `CMakeLists.txt` (install of the licenses, `CPACK_RPM_PACKAGE_LICENSE`, `-dev` requirements).

**Upstream**: <https://github.com/TsukiNi22/c2dmp-hsm/blob/main/LICENSE.md>, <https://github.com/TsukiNi22/s.o.s/blob/main/LICENSE.md>, `s.o.s/.github/workflows/sync.yml:66-67`, <https://creativecommons.org/licenses/by-nc-sa/4.0/>.

**Tools**: `audit-deps/scripts/deps.py`, `vulns.py`, `dnf updateinfo`, `gh api`.
