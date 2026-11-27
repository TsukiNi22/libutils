+++
title = "CI / CD"
summary = "The GitHub workflows of libutils: lint, unit tests (with ASan), packages, pre-releases and GitHub releases, publication on gh-pages and the downstream notification."
section = "resources"
order = 20
keywords = "ci cd workflow github actions dispatch lint shellcheck actionlint xstyle unit tests asan address sanitizer build packages release github release changelog pre-release tag build ignore gh-pages docs docker-image"
+++

The workflows of `.github/workflows/` test, build and publish the library on each push. The local commands are on [Build, tests and hooks](page:development).

## Workflows {#workflows}

:::html
<figure>
<svg viewBox="0 0 760 170" role="img" aria-label="Dispatch runs the unit tests, then builds the packages or the libraries, then notifies docker-image and creates the GitHub release of a tag">
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
<rect class="box-alt" x="590" y="104" width="150" height="50" rx="8"/>
<text class="lbl" x="665" y="134" text-anchor="middle">GitHub release</text>
<path class="edge" d="M150 85H186" marker-end="url(#arr4-fig7-en)"/>
<path class="edge" d="M320 78L366 48" marker-end="url(#arr4-fig7-en)"/>
<path class="edge dash" d="M320 92L366 124" marker-end="url(#arr4-fig7-en)"/>
<path class="edge" d="M540 41H586" marker-end="url(#arr4-fig7-en)"/>
<path class="edge" d="M540 58L586 120" marker-end="url(#arr4-fig7-en)"/>
<text class="small" x="345" y="56" text-anchor="middle">build</text>
<text class="small" x="345" y="122" text-anchor="middle">else</text>
<text class="small" x="578" y="84" text-anchor="middle">tag</text>
</svg>
<figcaption>The CI on every push of <code>main</code> or of a <code>v*</code> tag (<code>[ignore]</code> skips it); <code>Lint (CI)</code> and <code>Docs - Pages (CD)</code> run on their own.</figcaption>
</figure>
:::

| Workflow | Role |
|---|---|
| `Dispatch (CI/CD)` | Runs on every push (branch `main` or tag `v*`, not for `*.md` / `*.txt` / `docs/` only changes) and decides what to trigger from the ref / commit message; waits for the triggered workflows and fails with them |
| `Lint (CI)` | On every push of `main` (not with `[ignore]`): `shellcheck` on the shell scripts and the hook, `actionlint` on the workflows, `xstyle` on the whole repository (a `major` issue fails, `include/utils/algorithms` excluded like the hook); a tool that cannot be installed is only a warning |
| `Unit Tests - Libraries (CI)` | Builds and runs the unit tests (`ctest`, in parallel), always first: Linux in the `unit-tests` image (Debug, then the same tests with the AddressSanitizer), Windows (`build-unit-tests-windows`) built with MinGW-w64 and run with `wine` in the `mingw64-unit-tests` image (a test that hangs on exit is retried once) |
| `Build - Packages (CI/CD)` | Builds the packages (RPM / DEB / Arch in the `ci` image, `mingw64-libutils*` in the `build-packages-windows` job on `mingw64-ci`), signs them and publishes them on `gh-pages`; the publication waits for both jobs, a tag whose version is not `project(... VERSION)` is refused |
| `Release - GitHub (CD)` | On a `v*` tag, once its packages are published: the GitHub release (a pre-release for a `-pre` tag), its notes = the section of the tag in `CHANGELOG.md` (`[pre-release]` for a `-pre` tag without its own), the RPM / DEB / Arch packages attached |
| `Docs - Pages (CD)` | On a push of `main` changing `docs/`: copies the built pages (not the sources `docs/site/`) on `gh-pages` |
| `Build - Libraries (CI)` | Only builds the 3 libraries and checks they exist, when `Dispatch` decides **not** to build the packages |

> [!IMPORTANT]
> Every build depends on the result of the unit tests: if one fails, the pipeline stops.

> [!TIP]
> A commit containing `[ignore]` (preferably in its description) skips the whole CI.

## Releases {#releases}

| Trigger | Result |
|---|---|
| tag `vX.Y.Z` | stable packages and the GitHub release |
| tag `vX.Y.Z-<suffix>` (ex: `v4.3.1-pre`) | `-pre` packages (the 5 last versions of each are kept) and a GitHub pre-release |
| a commit with `[build]` | `-pre` packages |
| a commit with `[release]`, or any package build, once the unit tests passed | notification of `docker-image` |

Every release is kept in the repositories; a `-pre` package is replaced by any release of the same version or higher. The notes of a GitHub release come from the `CHANGELOG.md`: its text can be rewritten afterwards (`gh release edit`).

## Publication {#publication}

The packages are signed (GPG) and published on the `gh-pages` branch (`https://tsukini22.github.io/libutils`): `fedora/44/x86_64` (RPM, `createrepo_c`), `debian` (APT `stable main`) and `arch/x86_64` (pacman, `repo-add`: packages and database signed). Two publications never run at the same time (queued), and a push refused because `gh-pages` moved meanwhile is rebased and retried (5 times). The same branch serves this documentation in `docs/` (copied by `Docs - Pages (CD)`).

## Downstream {#downstream}

After the packages are published, `Dispatch (CI/CD)` notifies the `docker-image` repository (`repository_dispatch` `libutils-released`) to rebuild the `ci` and `mingw64-ci` images with the new packages: on every package build, or on a commit containing `[release]`, only once the unit tests passed.
