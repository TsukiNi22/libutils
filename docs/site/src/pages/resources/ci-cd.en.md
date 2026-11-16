+++
title = "CI / CD"
summary = "The GitHub workflows of libutils: unit tests, packages, pre-releases and releases, publication on gh-pages and the downstream notification."
section = "resources"
order = 20
keywords = "ci cd workflow github actions dispatch unit tests build packages release pre-release tag build ignore gh-pages docker-image"
+++

The workflows of `.github/workflows/` test, build and publish the library on each push. The local commands are on [Build, tests and hooks](page:development).

## Workflows {#workflows}

:::html
<figure>
<svg viewBox="0 0 760 170" role="img" aria-label="Dispatch runs the unit tests, then builds the packages or the libraries, then notifies docker-image">
<defs>
<marker id="arr4-fig7-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
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
<path class="edge" d="M150 85H186" marker-end="url(#arr4-fig7-en)"/>
<path class="edge" d="M320 78L366 48" marker-end="url(#arr4-fig7-en)"/>
<path class="edge dash" d="M320 92L366 124" marker-end="url(#arr4-fig7-en)"/>
<path class="edge" d="M540 41H586" marker-end="url(#arr4-fig7-en)"/>
<text class="small" x="345" y="56" text-anchor="middle">build</text>
<text class="small" x="345" y="122" text-anchor="middle">else</text>
</svg>
<figcaption>The CI on every push of <code>main</code> or of a <code>v*</code> tag (<code>[ignore]</code> skips it).</figcaption>
</figure>
:::

| Workflow | Role |
|---|---|
| `Dispatch (CI/CD)` | Runs on every push (branch `main` or tag `v*`, not for `*.md` / `*.txt` only changes) and decides what to trigger from the ref / commit message; waits for the triggered workflows and fails with them |
| `Unit Tests - Libraries (CI)` | Builds and runs the unit tests (`ctest`, in parallel), always first: Linux in the `unit-tests` image, Windows (`build-unit-tests-windows`) built with MinGW-w64 and run with `wine` in the `mingw64-unit-tests` image |
| `Build - Packages (CI/CD)` | Builds the packages (RPM / DEB / Arch in the `ci` image, `mingw64-libutils*` in the `build-packages-windows` job on `mingw64-ci`), signs them and publishes them on `gh-pages`; the publication waits for both jobs |
| `Build - Libraries (CI)` | Only builds the 3 libraries and checks they exist, when `Dispatch` decides **not** to build the packages |

> [!IMPORTANT]
> Every build depends on the result of the unit tests: if one fails, the pipeline stops.

> [!TIP]
> A commit containing `[ignore]` (preferably in its description) skips the whole CI.

## Releases {#releases}

| Trigger | Result |
|---|---|
| tag `vX.Y.Z` | stable packages |
| tag `vX.Y.Z-<suffix>` (ex: `v4.3.1-pre`) or a commit with `[build]` | `-pre` packages (the 5 last versions of each are kept) |
| a commit with `[release]`, or any package build | notification of `docker-image` |

Every release is kept in the repositories; a `-pre` package is replaced by any release of the same version or higher.

## Publication {#publication}

The packages are signed (GPG) and published on the `gh-pages` branch (`https://tsukini22.github.io/libutils`): `fedora/44/x86_64` (RPM, `createrepo_c`), `debian` (APT `stable main`) and `arch/x86_64` (pacman, `repo-add`: packages and database signed). Two publications never run at the same time (queued). The same branch serves this documentation in `docs/`.

## Downstream {#downstream}

After the packages are published, `Dispatch (CI/CD)` notifies the `docker-image` repository (`repository_dispatch` `libutils-released`) to rebuild the `ci` and `mingw64-ci` images with the new packages: on every package build, or on a commit containing `[release]`.
