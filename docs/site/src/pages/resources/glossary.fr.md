+++
title = "Glossaire"
summary = "Les mots de libutils et de cette documentation, avec la page qui explique chacun."
keywords = "glossaire mots définition section groupe define handle garde statut payload eetp slot ownership failsafe safe_mode type fixe sysroot"
+++

Les mots utilisés dans la bibliothèque et dans cette documentation.

## Bibliothèque {#library}

| Mot | Signification |
|---|---|
| Section | Une partie de la bibliothèque : un dossier de `include/utils/`, un namespace et un define (`_Network`). [Lier et inclure](page:usage#sections) |
| Groupe | Un define qui active plusieurs sections : `_Handling`, `_Tools`, `_Utils` |
| Code interne / externe | Un code d'exception de libutils (`InternalCode`) / de votre projet (`ExternalCode`). [Codes d'exception personnalisés](page:custom-exceptions) |
| Type (d'une exception) | Sa gravité : `None`, `Warning`, `Error`, `Fatal`, combinés en bits. [Exceptions](page:exception#type) |
| Restriction | Les types qu'un code accepte |
| `failsafe` | Un paramètre qui transforme une erreur en avertissement affiché |
| `safe_mode` | Un paramètre qui prend un verrou (appel thread safe) |
| Observer | Une classe de base qui suit les instances d'une classe ; le notificateur de fuites mémoire signale celles encore vivantes à la sortie. [Security](page:security#observer) |
| Migration / legacy | Une ancienne API gardée un temps (`_migration(x, y, z)`, supprimée vers cette version) ou pour de bon (`_legacy`) |

## Réseau et IPC {#network-ipc}

| Mot | Signification |
|---|---|
| Payload | Un message d'une connexion, terminé par le séparateur (`'\n'` par défaut). [Network](page:network#types) |
| EETP | Le protocole de `EETPParser` : un handshake RSA (`SYN`, `SO`) puis des messages AES-256-GCM. [Communication](page:network-ipc#eetp) |
| Clé RSA commune | La paire de clés partagée par les deux côtés d'EETP, `~/.ssh/common` et `~/.ssh/common.pub` |
| Slot | Une partie de taille fixe d'une mémoire partagée qui contient un message. [Encapsulation](page:encapsulation#shared-memory) |
| Ownership | L'id d'une instance de `SharedMemory` (son pid par défaut), utilisé pour adresser les messages |
| Target | Qui lit un message de la mémoire partagée : un ownership, ou tous les lecteurs |

## API C {#c-api}

| Mot | Signification |
|---|---|
| Handle | Un pointeur opaque vers un objet C++ (`utils_cli_t*`), créé par `_create` et libéré par `_destroy`. [Utiliser libutils depuis C](page:c-api#handles) |
| Garde | Le `try` / `catch` autour de chaque fonction C qui transforme une exception en valeur de retour et en statut. [Conception de l'API C](page:c-api-design#guard) |
| Statut | `utils_exception`, l'enregistrement thread local de la dernière erreur |
| Types fixes | Les types C pour lesquels un template est généré (`i8` ... `u64`, `size`, `f32`, `f64`, `f128`, `bool`, `char`, `str`). [Conception de l'API C](page:c-api-design#fixed-types) |
| Fonction `_r` | La version d'une fonction qui écrit sa chaîne dans votre buffer au lieu d'un buffer thread local |

## Build {#build}

| Mot | Signification |
|---|---|
| `-op` / `-db` / `-as` | La variante optimisée / debug / AddressSanitizer d'un paquet |
| `-pre` | Un paquet de pré-version |
| Sysroot | Le dossier des headers et bibliothèques Windows utilisés par MinGW-w64. [Windows](page:windows#requirements) |
| Toolchain | Le fichier CMake qui définit les compilateurs et le sysroot d'une compilation croisée |
