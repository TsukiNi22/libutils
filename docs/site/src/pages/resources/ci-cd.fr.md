+++
title = "CI / CD"
summary = "Les workflows GitHub de libutils : lint, tests unitaires (avec ASan), paquets, pré-versions et releases GitHub, publication sur gh-pages et notification en aval."
keywords = "ci cd workflow github actions dispatch lint shellcheck actionlint xstyle tests unitaires asan address sanitizer build paquets release github changelog pré-version tag build ignore gh-pages docs docker-image"
+++

Les workflows de `.github/workflows/` testent, compilent et publient la bibliothèque à chaque push. Les commandes locales sont sur [Build, tests et hooks](page:development).

## Workflows {#workflows}

:::html
<figure>
<svg viewBox="0 0 760 170" role="img" aria-label="Dispatch runs the unit tests, then builds the packages or the libraries, then notifies docker-image and creates the GitHub release of a tag">
<defs>
<marker id="arr4-fig7-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box-alt" x="20" y="60" width="130" height="50" rx="8"/>
<text class="lbl" x="85" y="90" text-anchor="middle">Dispatch</text>
<rect class="box" x="190" y="60" width="130" height="50" rx="8"/>
<text class="lbl" x="255" y="90" text-anchor="middle">Unit Tests</text>
<rect class="box" x="370" y="16" width="170" height="50" rx="8"/>
<text class="lbl" x="455" y="46" text-anchor="middle">Build Packages</text>
<rect class="box" x="370" y="104" width="170" height="50" rx="8"/>
<text class="lbl" x="455" y="134" text-anchor="middle">Build Libraries</text>
<rect class="box-alt" x="590" y="16" width="150" height="50" rx="8"/>
<text class="lbl" x="665" y="46" text-anchor="middle">docker-image</text>
<rect class="box-alt" x="590" y="104" width="150" height="50" rx="8"/>
<text class="lbl" x="665" y="134" text-anchor="middle">GitHub release</text>
<path class="edge" d="M150 85H186" marker-end="url(#arr4-fig7-fr)"/>
<path class="edge" d="M320 78L366 48" marker-end="url(#arr4-fig7-fr)"/>
<path class="edge dash" d="M320 92L366 124" marker-end="url(#arr4-fig7-fr)"/>
<path class="edge" d="M540 41H586" marker-end="url(#arr4-fig7-fr)"/>
<path class="edge" d="M540 58L586 120" marker-end="url(#arr4-fig7-fr)"/>
<text class="small" x="345" y="56" text-anchor="middle">build</text>
<text class="small" x="345" y="122" text-anchor="middle">else</text>
<text class="small" x="578" y="84" text-anchor="middle">tag</text>
</svg>
<figcaption>La CI à chaque push sur <code>main</code> ou d'un tag <code>v*</code> (<code>[ignore]</code> la saute) ; <code>Lint (CI)</code> et <code>Docs - Pages (CD)</code> tournent à part.</figcaption>
</figure>
:::

| Workflow | Rôle |
|---|---|
| `Dispatch (CI/CD)` | Tourne à chaque push (branche `main` ou tag `v*`, pas pour des changements uniquement sur des `*.md` / `*.txt` / `docs/`) et décide quoi déclencher selon la ref / le message de commit ; attend les workflows déclenchés et échoue avec eux |
| `Lint (CI)` | À chaque push sur `main` (pas avec `[ignore]`) : `shellcheck` sur les scripts shell et le hook, `actionlint` sur les workflows, `xstyle` sur tout le dépôt (un problème `major` échoue, `include/utils/algorithms` exclu comme dans le hook) ; un outil qui ne s'installe pas n'est qu'un avertissement |
| `Unit Tests - Libraries (CI)` | Compile et lance les tests unitaires (`ctest`, en parallèle), toujours en premier : Linux dans l'image `unit-tests` (Debug, puis les mêmes tests avec l'AddressSanitizer), Windows (`build-unit-tests-windows`) compilé avec MinGW-w64 et lancé avec `wine` dans l'image `mingw64-unit-tests` (un test bloqué à la sortie est relancé une fois) |
| `Build - Packages (CI/CD)` | Compile les paquets (RPM / DEB / Arch dans l'image `ci`, `mingw64-libutils*` dans le job `build-packages-windows` sur `mingw64-ci`), les signe et les publie sur `gh-pages` ; la publication attend les deux jobs, un tag dont la version n'est pas `project(... VERSION)` est refusé |
| `Release - GitHub (CD)` | Sur un tag `v*`, une fois ses paquets publiés : la release GitHub (une pré-version pour un tag `-pre`), ses notes = la section du tag dans `CHANGELOG.md` (`[pre-release]` pour un tag `-pre` sans la sienne), les paquets RPM / DEB / Arch attachés |
| `Docs - Pages (CD)` | Sur un push de `main` qui change `docs/` : copie les pages construites (pas les sources `docs/site/`) sur `gh-pages` |
| `Build - Libraries (CI)` | Compile seulement les 3 bibliothèques et vérifie qu'elles existent, quand `Dispatch` décide de **ne pas** compiler les paquets |

> [!IMPORTANT]
> Chaque build dépend du résultat des tests unitaires : si l'un échoue, le pipeline s'arrête.

> [!TIP]
> Un commit qui contient `[ignore]` (de préférence dans sa description) saute toute la CI.

## Releases {#releases}

| Déclencheur | Résultat |
|---|---|
| tag `vX.Y.Z` | paquets stables et la release GitHub |
| tag `vX.Y.Z-<suffix>` (ex : `v4.3.1-pre`) | paquets `-pre` (les 5 dernières versions de chacun sont gardées) et une pré-version GitHub |
| un commit avec `[build]` | paquets `-pre` |
| un commit avec `[release]`, ou tout build de paquets, une fois les tests unitaires passés | notification de `docker-image` |

Chaque release est gardée dans les dépôts ; un paquet `-pre` est remplacé par toute release de la même version ou d'une version supérieure. Les notes d'une release GitHub viennent du `CHANGELOG.md` : son texte peut être réécrit ensuite (`gh release edit`).

## Publication {#publication}

Les paquets sont signés (GPG) et publiés sur la branche `gh-pages` (`https://tsukini22.github.io/libutils`) : `fedora/44/x86_64` (RPM, `createrepo_c`), `debian` (APT `stable main`) et `arch/x86_64` (pacman, `repo-add` : paquets et base de données signés). Deux publications ne tournent jamais en même temps (mises en file d'attente), et un push refusé parce que `gh-pages` a bougé entre-temps est rebasé et relancé (5 fois). La même branche sert cette documentation dans `docs/` (copiée par `Docs - Pages (CD)`).

## En aval {#downstream}

Une fois les paquets publiés, `Dispatch (CI/CD)` notifie le dépôt `docker-image` (`repository_dispatch` `libutils-released`) pour reconstruire les images `ci` et `mingw64-ci` avec les nouveaux paquets : à chaque build de paquets, ou sur un commit qui contient `[release]`, seulement une fois les tests unitaires passés.
