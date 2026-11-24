# Workflows

Complete page: [CI / CD](https://tsukini22.github.io/libutils/docs/ci-cd.html).

| Workflow | Runs on | Does |
| -------- | ------- | ---- |
| `Dispatch (CI/CD)` | each push to `main` or a `v*` tag (not `*.md` / `*.txt` only) | picks the jobs from the ref / commit message |
| `Unit Tests - Libraries (CI)` | always first | Linux tests in `unit-tests`, Windows tests in `mingw64-unit-tests` under `wine` |
| `Build - Packages (CI/CD)` | a pre-release or a release | builds, signs and publishes the packages on `gh-pages` (`fedora/`, `debian/`, `arch/`) |
| `Build - Libraries (CI)` | when no package is built | checks that the libraries still compile, publishes nothing |

The Linux packages build in the `ci` image, the Windows `mingw64-libutils*` ones in the `build-packages-windows` job (`mingw64-ci`).

> [!TIP]
> A commit with `[ignore]` (best in its description) skips the whole CI; a tag push is never skipped.

## Pre-release (channel `unstable`)
- a tag `vX.Y.Z-<suffix>` (ex: `v5.0.1-pre`)
- or a commit with `[build]`, best in its description

> [!NOTE]
> The package repository keeps the 5 last versions of each `-pre` package, and every release.

## Release (channel `stable`)
- a tag `vX.Y.Z` only

## Downstream notification
`Dispatch (CI/CD)` notifies the `docker-image` repository after the packages are published (`repository_dispatch` `libutils-released`).
It rebuilds the `ci` and `mingw64-ci` images with the new packages. The secret `DOCKER_IMAGE_REPO_TOKEN` is shared with brain and xstyle.
- on every package build (release or pre-release)
- or on a commit with `[release]`, best in its description
