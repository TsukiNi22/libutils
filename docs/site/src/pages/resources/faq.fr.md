+++
title = "FAQ"
summary = "Les réponses aux problèmes courants : erreurs d'installation, un paquet manquant à la configuration, les defines, Windows, OK / KO, et l'API C."
keywords = "faq question problème erreur invalid hash paquet manquant find_package windows.h _unused OK KO link stdc++ api c posix sections define"
+++

Les problèmes les plus fréquents, avec leur réponse.

## Installation {#install}

**`dnf` / `apt` affiche `invalid hash` ?** Videz le cache de l'outil (`dnf clean all`, `apt clean`) ou relancez `setup.sh` ; si cela persiste, le dépôt est peut-être cassé : ouvrez une issue.

**Quel paquet pour quel type de build ?** `Debug` demande `libutils-db`, `Asan` demande `libutils-as`, tout le reste `libutils-op` (dans `libutils`). La configuration s'arrête avec le nom du paquet manquant. Voir [Lier et inclure](page:usage#cmake).

**Qu'est-ce qu'un paquet `-pre` ?** La pré-version de la prochaine version, compilée depuis `main` ; toute release de la même version ou d'une version supérieure la remplace.

## Compilation {#compile}

**Pourquoi une section que j'ai définie manque-t-elle ?** `utils.hpp` n'a pas de garde globale : les defines doivent être placés avant **chaque** include de ce header. Un include nu après un include partiel n'obtient que les sections déjà définies. Voir [Lier et inclure](page:usage#rules).

**`OK` / `KO` entrent-ils en conflit avec mes propres macros ?** libutils définit toujours `UTILS_OK` / `UTILS_KO` (utilisées dans la bibliothèque) et ne définit `OK` / `KO` que s'ils n'existent pas encore (`_UTILS_OK_DEFINED` / `_UTILS_KO_DEFINED` indiquent quand elle l'a fait) ; un avertissement est émis quand ils existent avec une autre valeur.

**Pourquoi un avertissement d'obsolescence sur `_migration` ?** L'API est renommée et sera supprimée à la version donnée dans l'avertissement : passez au nouveau nom, ou masquez ces avertissements avec `NO_BACKWARD_COMPATIBILITY_WARNING` ([Avertissements](page:warnings)).

## Windows {#windows}

**Des erreurs sur `_unused` avec `<windows.h>` ?** Le `<windows.h>` complet déclare un membre nommé `_unused`, une macro de libutils : incluez `<windows.h>` **avant** `<utils/utils.hpp>`, ou définissez `WIN32_LEAN_AND_MEAN`.

**Pourquoi `Process::spawn()` s'interrompt-il ?** Windows n'a pas de `fork` : utilisez `spawn(path, args)` (`CreateProcess`). Voir [Windows](page:windows#code).

**Pas de couleurs dans la console ?** Appelez `utils::iomanip::enable_ansi()` (C : `utils_iomanip_enable_ansi()`) une fois au démarrage.

## API C {#c-api}

**Des références indéfinies vers `std::` ou `operator new` au link ?** libutils est en C++ : liez avec un linker C++ (`LINKER_LANGUAGE CXX` en CMake) ou ajoutez `-lstdc++`. Voir [Utiliser libutils depuis C](page:c-api#setup).

**`pid_t` / `struct sockaddr` inconnus en C strict ?** Définissez `_POSIX_C_SOURCE 200809L` avant le premier include, ou compilez avec `-std=gnu11` / `gnu23`.

**Comment savoir pourquoi un appel a échoué ?** Lisez `utils_exception` dans le même thread, ou affichez `utils_exception_format()`. Voir [Erreurs](page:c-api#errors).

**Pourquoi mon callback a-t-il échoué avec un message générique ?** Renvoyez `utils_fail(code, "message")` au lieu de `UTILS_KO` : le message devient l'info de l'erreur.

**Puis-je passer n'importe quel handle à `utils_freezable_freeze` ?** Oui, n'importe quel handle de l'API C : il échoue avec `InvalidArgument` quand sa classe n'est pas un `Freezable`.
