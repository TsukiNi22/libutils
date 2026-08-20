# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

| Scope | Content |
| ----- | ------- |
| `In Progress` | Work that hasn't been completed yet (typically listed under [pre-release]) |
| `Added`       | New features, files, or sections introduced in this release |
| `Removed`     | Existing features, files, or sections that have been deleted |
| `Changed`     | Existing features or sections that were modified (behavior, structure, or content) |
| `Fixed`       | Bugs or issues that have been resolved |

> [!NOTE]
> The `(unofficial)` version are sub-version (such has fix typically) that don't have associated tag. The version often includes other sub-versions that weren't documented separately.
> A packaged builded at this version is not guaranteed!

---

## [pre-release] (empty)

## [v2.9.2] - 2026-08-20 (pre-release)
### Fixed
- [**MAJOR**] invalid namespace `utils::exceptionutils::exception` -> `utils::exception`

## [v2.9.0-pre] - 2026-08-20
### Added
- Unit tests for multiple sections (not all sections are covered, only: `security`, `verbose`)
- Documentation using github wiki (some sections might be missing or not finished)
- Add of a cluster class to handle groups of entity (new section `pool`)
- New class to encapsulation the shared object usage (new section `encapsulation`)
- Process encapsulation (Pipe **OK**, Dup **OK**, Process **OK**)

### Changed
- Rework the warning, now all are desactivated and can be activated/desactivated such has global/group/solo
- `vector` section moved to `math/vector`
- `middleware` section moved to `pool/middleware`

### Fixed
- The `math` section is now avaible on the `utils.hpp` include

## [v2.3.3] - 2026-08-16
### Added
- A parser class to handle formating and parsing of the network communication
- Network/Manip (Codec **OK**, Key **OK**, Parser **OK**, Socket **OK**, Client **OK**, Server **OK**, Testing **OK**)

### Fixed
- Fix 2etp parser with wrong framing
- Remove 

## v2.3.2 - 2026-08-16 (unofficial)
### Added
- Add custom output for verbose macro (mutex lock standart output -> global ouput)

### Fixed
- Multiple fix/testing for the network handling that will be on the official release for `v2.3.x`

## [v2.2.0] - 2026-08-01
### Changed
- **[MAJOR]** Rework the whole `observer` system
- The section `warning` became `security`

## [v2.1.0] - 2026-07-29
### Added
- Support for `.deb` along the existing `.rpm`
- Setup script that support any distro `fedora-like` & `debian-like`
- Rework of the file dispatching between packages (headers, cmake, ...)

## [v2.0.0] - 2026-07-28
### Changed
- **[MAJOR]** Change the way the lib is use, from file copy to `dnf` package handled
- The `write` section became the `manip` section with 2 sub-section `iomanip` & `smanip`
- All attribute where fixed to be used with other include `<name>` -> `_<name>`
- All exception are writted on cerr (was on cout before)

### Added
- `s.o.s` Algorithm
- Add the codec class to handle encoding/decoding of string
- Add of a fatal exception that can't be catch
- Better exception handling (link to the file, module name detection, ...)

### Removed
- Auto sync with cpp_project_template

## [v1.0.0] - 2026-07-17
### Added
- Initial version of the utils library with multiple tools (see [README-1.0.0](https://github.com/TsukiNi22/libutils/blob/v1.0.0/README.md) for more details)

[pre-release]: https://github.com/TsukiNi22/libutils/compare/v2.9.2-release...HEAD
[v2.9.2]: https://github.com/TsukiNi22/libutils/compare/v2.9.0-pre...v2.9.2-release
[v2.9.0-pre]: https://github.com/TsukiNi22/libutils/compare/v2.3.3-release...v2.9.0-pre
[v2.3.3]: https://github.com/TsukiNi22/libutils/compare/v2.2.0...v2.3.3-release
[v2.2.0]: https://github.com/TsukiNi22/libutils/compare/v2.1.0...v2.2.0
[v2.1.0]: https://github.com/TsukiNi22/libutils/compare/v2.0.0...v2.1.0
[v2.0.0]: https://github.com/TsukiNi22/libutils/compare/v1.0.0...v2.0.0
[v1.0.0]: https://github.com/TsukiNi22/libutils/releases/tag/v1.0.0

---
