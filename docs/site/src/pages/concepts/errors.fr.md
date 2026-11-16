+++
title = "Erreurs"
summary = "Comment libutils signale les erreurs : une exception avec un type et un code en C++, une valeur de retour et un statut thread local en C, et les codes générés depuis du JSON."
keywords = "erreurs exception type code restriction failsafe interne externe json généré statut utils_exception frontière C"
+++

Chaque erreur de libutils est une exception qui porte un **type** et un **code**, jamais un code de retour. L'API C transforme la même exception en valeur de retour et en statut. Les classes et la liste complète des codes sont sur [Exceptions](page:exception).

## Type et code {#type-code}

Le **type** dit la gravité, le **code** ce qui s'est passé :

| Classe | Type | Signification |
|---|---|---|
| `NoneException` | `None` | pas une erreur (ex : aide affichée, `Exit`) |
| `WarningException` | `Warning` | le programme peut continuer |
| `ErrorException` | `Error` | erreur de l'appel |
| `CustomException` | donné | une combinaison de types |
| `FatalException` | `Fatal` | affiche et appelle `std::abort()` depuis son constructeur |

Un code peut restreindre les types qu'il accepte : une combinaison interdite finit en `FatalException(ExceptionCodeRestriction)`. Beaucoup de méthodes prennent `failsafe` (avertir au lieu de lever une exception) ou `safe_mode` (verrou).

```cpp
try {
    server.start();
} catch (const utils::exception::IException& e) {
    if (e.getCode() == utils::exception::InternalCode::AlreadyRunning) return;
    std::cerr << e.formated() << std::endl; // [Error] file:line (origin) -> message / function = info
}
```

## D'où viennent les codes {#codes}

Les codes sont générés au build depuis `cmake/config/exceptions/**/*.json` (`generated_internal_exception_header.hpp`) : `InternalCode` pour la bibliothèque. Un projet ajoute ses propres codes, `ExternalCode`, avec le même format JSON et le même générateur ([Codes d'exception personnalisés](page:custom-exceptions)). Le générateur refuse un code qui n'est pas un identifiant C++, `Undefined` ou un doublon.

| Origine | C++ | C (`utils_exception.origin`) |
|---|---|---|
| un code de libutils | `InternalCode` | `C_API_EXCEPTION_ORIGIN_INTERNAL` |
| un code de votre projet | `ExternalCode` | `C_API_EXCEPTION_ORIGIN_EXTERNAL` |
| une exception standard | `std::exception` | `C_API_EXCEPTION_ORIGIN_STD` (code `C_API_OTHER_EXCEPTION_CODE`) |
| tout autre objet levé | - | `C_API_EXCEPTION_ORIGIN_UNKNOWN` |
| une fonction C (`utils_fail`) | - | `C_API_EXCEPTION_ORIGIN_C` |

## À travers la frontière C {#c-boundary}

Une exception ne doit jamais passer dans du code C (comportement indéfini). Chaque fonction de l'API C exécute son code C++ dans une **garde** :

1. l'appel tourne dans un `try` ;
2. en cas de succès, seul `utils_exception.exception` est mis à `false` (aucune copie de chaîne sur le chemin rapide) ;
3. sur une exception, la garde remplit le `utils_exception` thread local (type, code, what, info, file, line, function, module, origin) et la fonction renvoie `UTILS_KO`, `NULL` ou `0`.

Dans l'autre sens, un callback C qui échoue renvoie `UTILS_KO` (ou `utils_fail(code, info)`) : le côté C++ lève une exception avec cette info, qui remonte comme n'importe quelle autre et finit dans le statut de l'appel qui a lancé le callback. Détails : [Conception de l'API C](page:c-api-design#guard).

:::html
<figure>
<svg viewBox="0 0 760 270" role="img" aria-label="A C program calls the C functions, each one runs the C++ classes in a guard; a failure fills the thread local status utils_exception; a C callback called by the C++ code returns UTILS_OK or utils_fail">
<defs>
<marker id="arrc-fig2-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
<marker id="arrca-fig2-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow-accent" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box" x="20" y="40" width="150" height="50" rx="8"/>
<text class="lbl" x="95" y="70" text-anchor="middle">C program</text>
<rect class="box-alt" x="210" y="40" width="160" height="50" rx="8"/>
<text class="lbl" x="290" y="62" text-anchor="middle">utils.h · CApi.h</text>
<text class="small" x="290" y="80" text-anchor="middle">extern "C" functions</text>
<rect class="box-alt" x="410" y="40" width="150" height="50" rx="8"/>
<text class="lbl" x="485" y="62" text-anchor="middle">guard</text>
<text class="small" x="485" y="80" text-anchor="middle">try / catch</text>
<rect class="box" x="600" y="40" width="140" height="50" rx="8"/>
<text class="lbl" x="670" y="62" text-anchor="middle">C++ classes</text>
<text class="small" x="670" y="80" text-anchor="middle">in the handles</text>
<path class="edge" d="M170 65H206" marker-end="url(#arrc-fig2-fr)"/>
<path class="edge" d="M370 65H406" marker-end="url(#arrc-fig2-fr)"/>
<path class="edge" d="M560 65H596" marker-end="url(#arrc-fig2-fr)"/>
<rect class="group" x="330" y="140" width="230" height="44" rx="8"/>
<text class="lbl" x="445" y="167" text-anchor="middle">utils_exception (thread local)</text>
<path class="edge dash" d="M485 90V136" marker-end="url(#arrc-fig2-fr)"/>
<text class="small" x="492" y="118">exception</text>
<path class="edge dash" d="M330 162H95V94" marker-end="url(#arrc-fig2-fr)"/>
<text class="small" x="105" y="154">UTILS_KO / NULL, read like errno</text>
<rect class="box" x="20" y="210" width="150" height="44" rx="8"/>
<text class="lbl" x="95" y="237" text-anchor="middle">C callback</text>
<path class="edge-accent" d="M690 90V232H174" marker-end="url(#arrca-fig2-fr)"/>
<text class="small" x="430" y="224" text-anchor="middle">fn(..., data) → UTILS_OK / utils_fail(code, info)</text>
</svg>

<figcaption>Une exception du code C++ s'arrête au guard de la fonction C : elle remplit <code>utils_exception</code> et la fonction renvoie sa valeur d'erreur ; un callback C qui échoue fait le chemin inverse.</figcaption>
</figure>
:::

## Sortie formatée {#output}

`formated()` en C++, `utils_exception_format()` en C, donnent le même texte, construit par `utils::exception::format` :

```text
[<type>] ...<file_path:hyperlink>:<line> (<origin>) -> <message>
-------------------------------------------
<function_demangled> = <info>
-------------------------------------------
```
