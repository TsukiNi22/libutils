+++
title = "Algorithms"
summary = "c2dmp-hsm (distance entre deux chaînes) et s.o.s (stéganographie), synchronisés depuis leurs propres dépôts sous CC BY-NC-SA 4.0."
keywords = "algorithmes c2dmp hsm distance similarité chaîne sos stéganographie cacher extraire porteuse payload bruit utils_c2dmp utils_sos_embed utils_sos_extract"
+++

| Define | Groupe | Namespaces | Headers |
|---|---|---|---|
| `_Algorithms` | `_Tools` | `utils::algorithms::c2dmp`, `utils::algorithms::sos` | `utils/algorithms/` |
| `_C2DMP` / `_SOS` (depuis `_Algorithms`) | `_Algorithms` | les mêmes | `utils/algorithms/c2dmp-hsm/`, `utils/algorithms/sos/` |

> [!CAUTION]
> Ces algorithmes sont synchronisés depuis [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) et [s.o.s](https://github.com/TsukiNi22/s.o.s) et sont sous **CC BY-NC-SA 4.0** (pas la licence MIT de libutils) : pas d'usage commercial sans accord avec l'auteur ([Licences](page:licenses)).

## c2dmp-hsm {#c2dmp-hsm}

`_C2DMP` : distance entre deux mots, plus elle est petite plus ils sont proches. Insensible à la casse et aux accents Latin-1, temps linéaire et mémoire constante. Utilisé pour les suggestions du [Cli](page:cli).

```cpp
template<std::uint_fast8_t prefixDepthSearch = 3, typename UIntT = std::uint_fast8_t, bool full = false>
float c2dmp(std::string_view a, std::string_view b);
```

| Paramètre | Description |
|---|---|
| `prefixDepthSearch` | Profondeur de la recherche du préfixe commun |
| `UIntT` | Type du compteur (`std::uint_fast8_t` jusqu'à 255 caractères, des types plus grands pour des chaînes plus longues) |
| `full` | `true` : compte les caractères mal placés de tout `a` (`c2dmp_foptimized`), sinon `c2dmp_optimized` |

Variantes : `c2dmp_optimized`, `c2dmp_foptimized`, `c2dmp_simplified`, `c2dmp_fsimplified` (les `simplified` sont dépréciées).

```cpp
#define _C2DMP
#include <utils/utils.hpp>

std::vector<std::string> commands = {"help", "exit", "history"};
std::string typed = "exot";
std::string closest = *std::min_element(commands.begin(), commands.end(), [&](const std::string& a, const std::string& b) {
    return utils::algorithms::c2dmp::c2dmp(typed, a) < utils::algorithms::c2dmp::c2dmp(typed, b);
}); // "exit"
```

## s.o.s {#sos}

`_SOS` : **S**teganography **O**ptimized and **S**ecurized, cache un payload dans un signal (ex : les échantillons d'un fichier audio) avec une modification imperceptible, éventuellement avec une clé.

| Fonction | Description |
|---|---|
| `sos_embed<options, magic>(carrier, payload)` / `(carrier, payload, key)` | Cache `payload` dans `carrier` (modifié sur place) |
| `sos_extract<magic>(carrier)` / `(carrier, key)` | Récupère le payload |
| `tools::to_bytes<ByteT>(range)` / `tools::bytes_to<Range>(bytes)` | Convertit une chaîne / un range vers le type d'échantillon (`Byte` = `std::uint16_t` par défaut) et inversement (le résultat peut finir par du padding) |

| Option | Effet |
|---|---|
| `Option::None` | Pas de bruit, seuls les échantillons utilisés pour cacher les données changent |
| `Option::Noise` | Bruit généré sur chaque échantillon avant de cacher les données |
| `Option::GlobalNoise` | Bruit généré seulement sur les échantillons dans l'amplitude acceptée (ceux qui peuvent contenir des données) |

> [!WARNING]
> Le payload peut utiliser au plus 7,5 % du signal et la porteuse a besoin d'assez d'échantillons valides : sinon `std::out_of_range` ; extraire d'une porteuse sans message caché lève `std::invalid_argument` (les algorithmes gardent les exceptions standard de leur dépôt).

```cpp
#define _SOS
#include <utils/utils.hpp>

std::vector<std::uint16_t> carrier = /* 16 bits samples */;
std::vector<std::uint16_t> payload = utils::algorithms::sos::tools::to_bytes<std::uint16_t>(std::string("secret"));
utils::algorithms::sos::sos_embed(carrier, payload, key);
std::string message = utils::algorithms::sos::tools::bytes_to<std::string>(utils::algorithms::sos::sos_extract(carrier, key));
```

## En C {#in-c}

L'API C est un wrapper de libutils, en dehors des sources synchronisées. Les paramètres de template deviennent des arguments lus à l'exécution.

| C++ | C |
|---|---|
| `c2dmp<depth, UIntT, full>(a, b)` | `utils_c2dmp(a, a_size, b, b_size, prefix_depth, counter_size, full)` : `prefix_depth` de 1 à 5, `counter_size` 1, 2, 4 ou 8 octets, `-1` en cas d'erreur |
| `c2dmp(a, b)` (valeurs par défaut) | `utils_c2dmp_default(a, b)` (chaînes terminées par `'\0'`) |
| `sos_embed<options>(carrier, payload, key)` | `utils_sos_embed_<name>(carrier, count, payload, payload_count, key, key_count, options)`, `<name>` = `u8`, `u16`, `u32`, `u64`, `options` = `C_API_SOS_OPTION_*` combinées avec `\|` |
| `sos_extract(carrier, key)` | `utils_sos_extract_<name>(carrier, count, key, key_count, out, max)` : renvoie le nombre complet d'éléments du payload (`0` en cas d'erreur) |

Le magic des payloads cachés est fixé à `C_API_SOS_MAGIC` (`0x22`) en C. Les exceptions standard des algorithmes sont signalées dans `utils_exception` avec l'origine `C_API_EXCEPTION_ORIGIN_STD`.

```c
#define _C2DMP
#include <utils.h>
#include <stdio.h>

int main(void)
{
    const char* commands[] = {"help", "exit", "history"};
    const char* closest = commands[0];

    for (size_t i = 1; i < 3; ++i)
        if (utils_c2dmp_default("exot", commands[i]) < utils_c2dmp_default("exot", closest))
            closest = commands[i];
    printf("%s\n", closest); // exit
    return 0;
}
```
