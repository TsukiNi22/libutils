# Install

## Setup script
`setup.sh` registers the libutils package repository, then the system package manager installs the packages.
It detects the distribution from `/etc/os-release` (`ID`, then `ID_LIKE`):

| Distribution | Steps |
| ------------ | ----- |
| `rpm` (Fedora, RHEL) | adds `/etc/yum.repos.d/libutils.repo`, imports the GPG key with `rpm --import` |
| `deb` (Debian, Ubuntu) | adds the key in `/etc/apt/keyrings/libutils.gpg`, the source in `/etc/apt/sources.list.d/libutils.list`, runs `apt-get update` |
| `arch` (Arch) | adds the key to the pacman keyring (`--add`, `--lsign-key`), `[libutils]` in `/etc/pacman.conf`, runs `pacman -Sy` |

| Argument | Effect |
| -------- | ------ |
| `--no-sudo` | No `sudo` (run it as root) |
| `-h`, `--help` | Display the usage |

> [!NOTE]
> The script downloads the key and the repository files with `curl`, even when it is fetched with `wget`.

## Known errors

> [!WARNING]
> `invalid hash` from `dnf` / `apt`: clear its cache, then run `setup.sh` again.

If the error stays, the package repository itself is broken: open an issue.
