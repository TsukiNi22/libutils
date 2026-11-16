+++
title = "Cli"
summary = "Une ligne de commande interactive personnalisable : commandes, prompt, historique, flèches, suggestions, auto-complétion, opérateurs logiques, dans le thread courant ou dans un thread."
keywords = "cli commande ligne interactive prompt historique suggestion complétion flags TERM2 setCommand start interrupt kill hooks middlewares code de sortie utils_cli_create"
+++

| Define | Groupe | Espace de noms | En-têtes |
|---|---|---|---|
| `_Cli` | `_Handling` | `utils::cli` | `utils/cli/Cli.hpp`, `utils/cli/Flags.hpp` |

Une ligne de commande interactive personnalisable : commandes, prompt, historique, flèches, suggestions, auto-complétion, opérateurs logiques, exécutée dans le thread courant ou dans un thread. Le retour à la ligne automatique n'est pas géré.

## Flags {#flags}

Les valeurs de `utils::cli::Flag` se combinent avec `|` et se règlent avec `setFlags`, `addFlags`, `subFlags`, `resetFlags` (retour à `flags::DEFAULT`).

| Flag | Effet |
|---|---|
| `DEBUG` | Verbose pour les actions internes (rien pour l'instant) |
| `NOECHO` | N'affiche pas l'écho de la saisie |
| `CATCH` | Attrape les exceptions des commandes (code `130`) au lieu de les laisser remonter |
| `EMPTY_INPUT` | Ignore les saisies vides (par défaut : erreur `127`) |
| `TRIM` | Retire les espaces autour de la saisie |
| `PARSED` | Découpe la saisie et appelle les commandes parsées (`std::vector<std::string>`, par défaut : `std::string` brute) |
| `PROMPT` | Affiche le prompt (seulement sur un tty) |
| `LOGIC` | Active `&&`, `\|\|` et `;` entre les commandes (sémantique de bash) |
| `ARROW` | Flèches gauche / droite dans la saisie |
| `HISTORY` | Historique avec les flèches haut / bas |
| `PERSISTENT` | Garde l'historique entre les sessions (`~/.utils-cli_history`, 10 000 lignes max) |
| `HINT` | Affiche une suggestion quand une commande échoue |
| `AUTO_COMPLETION` | Complète la première commande avec <kbd>Tab</kbd> |
| `MANUAL` | Ne traite que `call` saisies par `start` (voir [Démarrage et arrêt](#start-stop)) |
| `THREAD` | S'exécute dans un thread, `start` le renvoie |
| `DETACHED` | Détache le thread (`start` ne renvoie rien, attendre avec `join()`) |
| `NO_TTY` | Autorise l'utilisation sans tty (par défaut : erreur `CliTTY`) |

| Preset (`utils::cli::flags::`) | Contenu |
|---|---|
| `DUMB` | rien |
| `DEFAULT` | `CATCH`, `EMPTY_INPUT`, `TRIM`, `PROMPT`, `ARROW` |
| `TERM1` | `DEFAULT` + `PARSED`, `LOGIC`, `HISTORY` |
| `TERM2` | `TERM1` + `HINT`, `AUTO_COMPLETION` |
| `TERM3` | `TERM2` + `THREAD` |
| `LOG` | `TERM3` + `DETACHED`, `NO_TTY` |
| `DEV` | `TERM2` + `DEBUG` |
| `MULTI_THREADING` | `THREAD`, `DETACHED` |
| `ALL` | tous les flags sauf `PERSISTENT` et `NO_TTY` |

## Commandes {#commands}

| Genre | Signature | Utilisée quand |
|---|---|---|
| brute | `std::function<void(const Cli&, const std::string& input)>` | `PARSED` désactivé |
| parsée | `std::tuple<std::function<void(const Cli&, const std::vector<std::string>& args)>, std::int16_t min, std::int16_t max>` | `PARSED` activé, `args[0]` est la commande, `min` / `max` comptent les autres arguments (`-1` = sans limite) |

| Méthode | Description |
|---|---|
| `setCommand<force>(name, command)` / `setCommands<force>(map)` | Ajoute des commandes, `force = true` pour écraser une commande existante |
| `delCommand(name)` / `delCommands(names)` | Supprime des commandes |
| `clearCommands()` | Supprime toutes les commandes |
| `resetCommands()` | Revient aux seules commandes intégrées |

Commandes intégrées : `help` (liste des commandes), `exit` / `quit` / `bye` (arrête la cli), `?` (dernier code et sa signification).

## Démarrage et arrêt {#start-stop}

| Méthode | Description |
|---|---|
| `start(call = 1, failsafe = false)` | Lance la cli ; renvoie le thread avec `THREAD` (sans `DETACHED`), sinon `std::nullopt` |
| `start(input, ...)` / `start(inputs, ...)` | Pareil, en exécutant d'abord les saisies données |
| `interrupt()` | S'arrête après la saisie en cours (comme <kbd>Ctrl</kbd>+<kbd>C</kbd> quand le constructeur reçoit `sig = true`) |
| `kill()` | S'arrête, ne peut plus être relancée |
| `join()` | Attend l'arrêt de la cli (thread détaché) |
| `isRunning()`, `wasInterrupted()`, `wasKilled()`, `wasStopped()` | État |
| `getCode()` / `strcode(code)` | Dernier code et son texte |
| `getHistory()` | Historique des saisies |
| `setInputDelimitor(c)` | Caractère de fin de saisie (par défaut `'\n'`) |

> [!NOTE]
> `Cli(true)` attrape <kbd>Ctrl</kbd>+<kbd>C</kbd> / <kbd>Ctrl</kbd>+<kbd>Z</kbd> (les handlers précédents sont restaurés par le destructeur). Le terminal passe en mode raw pendant que la cli tourne, seulement quand stdin est un tty.

> [!NOTE]
> Windows : la console est réglée via `utils::platform::tcgetattr` / `tcsetattr` (pas de saisie par ligne, pas d'écho, entrée et sortie VT) ; <kbd>Ctrl</kbd>+<kbd>Z</kbd> est un caractère (pas de `SIGTSTP`), l'historique est dans `%USERPROFILE%` quand `HOME` n'est pas défini.

## Hooks {#hooks}

| Hook | Définir / réinitialiser | Défaut |
|---|---|---|
| prompt `void(const Cli&, std::uint8_t code)` | `setPromptHook` / `resetPromptHook` | `defaultPromptHook` : affiche `> ` |
| parser `ParsedData(const std::string&, bool trim, bool logic, bool parse)` | `setParserHook` / `resetParserHook` | `defaultParserHook` : guillemets, échappements, `&&` / `\|\|` / `;` |
| getc `bool(char& c)` | `setGetCHook` / `resetGetCHook` | `defaultGetCHook` : lit stdin (renvoie `false` quand rien n'a été lu) |

`resetHooks()` réinitialise les 3 hooks.

## Middlewares {#middlewares}

Membres publics de type [middleware](page:pool#middleware), réinitialisés avec `resetMiddlewares()` :

| Membre | Arguments de `before` / `after` | Appelés |
|---|---|---|
| `cliMiddlewares` | - / - | début / fin de la cli |
| `errorMiddlewares` | code / code | quand un code d'erreur est défini |
| `promptMiddlewares` | - / - | autour du prompt |
| `inputMiddlewares` | - / `char` | après chaque touche (seulement `after`) |
| `parserMiddlewares` | saisie / données parsées | autour du parser |
| `execMiddlewares` | données parsées / données parsées | autour de l'exécution |
| `commandMiddlewares` | saisie / saisie | autour de chaque commande |

## Codes de sortie {#exit-codes}

| Code | Signification |
|---|---|
| `0` | OK |
| `1` | Erreur interne |
| `2` | Erreur interne d'un hook ou hook manquant |
| `3` | Erreur interne d'un middleware |
| `124` | Pas assez d'arguments |
| `125` | Trop d'arguments |
| `126` | Séquence d'échappement non fermée |
| `127` | Saisie vide |
| `128` | Commande inconnue |
| `129` | Commande non implémentée |
| `130` | Exception dans un callback |
| `255` | Erreur non définie |

## Exemple {#example}

```cpp
#define _Cli
#include <utils/utils.hpp>

int main(void)
{
    utils::cli::Cli cli(true); // catch ctrl-c & ctrl-z
    cli.setFlags(utils::cli::flags::TERM2);
    cli.setCommand("add", std::make_tuple(
        std::function<void(const utils::cli::Cli&, const std::vector<std::string>&)>(
            [](const utils::cli::Cli&, const std::vector<std::string>& args) {
                std::cout << std::stoi(args[1]) + std::stoi(args[2]) << std::endl;
            }),
        std::int16_t{2}, std::int16_t{2} // exactly 2 arguments
    ));
    (void)cli.start();
    return cli.getCode();
}
```

## En C {#in-c}

Chaque appel de `Cli` passe par le handle opaque `utils_cli_t*` (`utils_cli_create(sig)` / `utils_cli_destroy`). Le thread de `THREAD` est gardé dans le handle : attendez-le avec `utils_cli_join`.

| C++ | C |
|---|---|
| `Cli(sig)` | `utils_cli_create(sig)` (`NULL` en cas d'erreur) / `utils_cli_destroy(cli)` (tue et attend une cli en cours) |
| `setFlags(flags::TERM2)` | `utils_cli_set_flags(cli, C_API_CLI_FLAGS_TERM2)` (`C_API_CLI_FLAG_*` combinés avec `\|`) |
| commande parsée | `utils_cli_set_parsed_command(cli, name, fn, min, max, data, force)`, `fn(cli, argc, argv, data)` avec `argv[0]` = la commande |
| commande brute | `utils_cli_set_raw_command(cli, name, fn, data, force)` |
| `start(inputs, call)` | `utils_cli_start(cli, inputs, count, call)` |
| hooks, middlewares | `utils_cli_set_*_hook`, `utils_cli_<name>_middlewares_add_before` / `_add_after` / `_clear` |
| `strcode(code)`, `getHistory()` | `utils_cli_strcode` / `utils_cli_strcode_r`, `utils_cli_history_get` / `_r`, `utils_cli_history_size` |
| `ParsedData` | `utils_cli_parsed_t*` (`utils_cli_parsed_*`) |

Un callback renvoie `UTILS_OK`, `UTILS_KO` (avec `utils_fail(code, info)` pour le message) ou `C_API_CLI_EXIT` pour arrêter la cli comme la commande `exit`.

```c
#define _Cli
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>

static int add(const utils_cli_t* cli, size_t argc, const char* const* argv, void* data)
{
    (void)cli;
    (void)argc;
    (void)data;
    printf("%ld\n", strtol(argv[1], NULL, 10) + strtol(argv[2], NULL, 10));
    return UTILS_OK;
}

int main(void)
{
    utils_cli_t* cli = utils_cli_create(true);   // catch ctrl-c & ctrl-z
    int code = 1;

    if (!cli)
        return 1;
    utils_cli_set_flags(cli, C_API_CLI_FLAGS_TERM2);
    if (utils_cli_set_parsed_command(cli, "add", add, 2, 2, NULL, false) == UTILS_OK
        && utils_cli_start(cli, NULL, 0, 1) == UTILS_OK)
        code = utils_cli_get_code(cli);
    else
        fprintf(stderr, "%s\n", utils_exception_format());
    utils_cli_destroy(cli);
    return code;
}
```
