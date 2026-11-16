+++
title = "Licences"
summary = "libutils est sous MIT ; les algorithmes synchronisés sont sous CC BY-NC-SA 4.0 et OpenSSL sous Apache-2.0."
keywords = "licence licences mit cc by-nc-sa apache openssl mentions tierces commercial algorithmes c2dmp sos"
+++

La licence MIT de libutils ne couvre que son propre code : chaque dépendance, incluse ou liée, garde sa propre licence.

## Composants {#components}

| Composant | Licence | Utilisation |
|---|---|---|
| libutils (son propre code) | MIT ([LICENSE](https://github.com/TsukiNi22/libutils/blob/main/LICENSE)) | - |
| `include/utils/algorithms/` ([c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm), [s.o.s](https://github.com/TsukiNi22/s.o.s)) | CC BY-NC-SA 4.0 ([LICENSE.md](https://github.com/TsukiNi22/libutils/blob/main/include/utils/algorithms/LICENSE.md)) | synchronisés depuis leurs dépôts |
| [OpenSSL](https://www.openssl.org) | Apache-2.0 | lié (paquet système) |

> [!CAUTION]
> Les algorithmes synchronisés ne peuvent pas être utilisés commercialement sans accord avec l'auteur. L'API C des algorithmes (`utils/algorithms/CApi.h`) est du code de libutils, mais elle les appelle : la même restriction s'applique à un programme qui l'utilise.

La liste complète est dans [THIRD_PARTY_NOTICES.md](https://github.com/TsukiNi22/libutils/blob/main/THIRD_PARTY_NOTICES.md) ; les paquets installent les licences dans `share/licenses/libutils/`.
