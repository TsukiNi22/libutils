# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]
### In Progress
- A parser class to handle formating and parsing of the network communication
- Network (Codec **OK**, Parser **KO**, Key **KO**, Client **KO**, Server **KO**)

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

[Unreleased]: https://github.com/TsukiNi22/libutils/compare/v2.1.0...HEAD
[v2.1.0]: https://github.com/TsukiNi22/libutils/compare/v2.0.0...v2.1.0
[v2.0.0]: https://github.com/TsukiNi22/libutils/compare/v1.0.0...v2.0.0
[v1.0.0]: https://github.com/TsukiNi22/libutils/releases/tag/v1.0.0

---
