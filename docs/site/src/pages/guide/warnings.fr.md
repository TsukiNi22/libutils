+++
title = "Avertissements"
summary = "Activer ou désactiver les avertissements de compilation de libutils, tous ensemble, par domaine ou un par un."
keywords = "avertissement _Warning _NoWarning DEPRECATED_WARNING BACKWARD_COMPATIBILITY_WARNING LINKER_WARNING USAGE_WARNING NO_ migration obsolète"
+++

Par défaut, tous les avertissements de la bibliothèque sont désactivés, sauf ceux qui sont forcés (un fichier header manquant, un avertissement fatal). Ils se choisissent avec des defines, avant l'include ou pour tout le projet (`add_definitions(-D_NoWarning)`).

## Tous {#all}

| Define | Effet |
|---|---|
| `_Warning` | Tous les avertissements |
| `_NoWarning` | Aucun avertissement |

Chaque define ci-dessous a une version de désactivation, `NO_<define>`. Les noms suivent un seul modèle :

```text
domain warning (group):        <domain_name>_WARNING
esoteric warning (individual): <thing_name>_<domain_name>_WARNING
warning disable:               NO_<warning>
```

## Domaines {#domains}

| Nom | Define | Avertit sur |
|---|---|---|
| Obsolète | `DEPRECATED_WARNING` | L'usage de quelque chose d'obsolète (seulement avec l'attribut `_deprecated(info)`) |
| Rétrocompatibilité | `BACKWARD_COMPATIBILITY_WARNING` | L'usage d'une API renommée / ancienne gardée pour la compatibilité (`_migration(x, y, z)`, `_legacy`) |
| Linker | `LINKER_WARNING` | Les dépendances nécessaires au link, comme `-lssl -lcrypto` pour le chiffrement |
| Usage | `USAGE_WARNING` | Les choses à utiliser avec précaution (promesses nécessaires pour les choses personnalisées...) |

## Un par un {#individual}

| Nom | Domaine | Define |
|---|---|---|
| Base64 Codec | Linker | `BASE64CODEC_LINKER_WARNING` |
| RSA Key | Linker | `RSAKEY_LINKER_WARNING` |
| AES Key | Linker | `AESKEY_LINKER_WARNING` |
| Fatal Exception | Usage | `FATALEXCEPTION_USAGE_WARNING` |
| EETP Parser | Usage | `EETPPARSER_USAGE_WARNING` |

## Exemple {#example}

```cpp
#define _Warning                            // everything
#define NO_BACKWARD_COMPATIBILITY_WARNING   // except the migration warnings
#include <utils/utils.hpp>
```

En C, `<utils.h>` connaît `_Warning` / `_NoWarning` pour les avertissements d'obsolescence (`DEPRECATED_WARNING`, `BACKWARD_COMPATIBILITY_WARNING`).
