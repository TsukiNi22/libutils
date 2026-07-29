#!/bin/bash
set -euo pipefail

BASE_URL="https://tsukini22.github.io/libutils"
SUDO="sudo"

# =========================
# Parse arguments
# =========================
usage() {
    echo "Usage: $0 [--no-sudo]"
    echo "  --no-sudo   Run without sudo (requires the script to be run as root already)"
    exit 1
}
 
for arg in "$@"; do
    case "$arg" in
        --no-sudo)
            SUDO=""
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Error: unknown argument '$arg'" >&2
            usage
            ;;
    esac
done

# =========================
# Find the os
# =========================
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS_ID="${ID:-unknown}"
    OS_ID_LIKE="${ID_LIKE:-}"
else
    echo "Error: /etc/os-release not found, cannot detect OS." >&2
    exit 1
fi

is_rpm_based() {
    [[ "$OS_ID" =~ ^(fedora|rhel|centos|rocky|almalinux)$ ]] || [[ "$OS_ID_LIKE" =~ (fedora|rhel) ]]
}

is_deb_based() {
    [[ "$OS_ID" =~ ^(debian|ubuntu)$ ]] || [[ "$OS_ID_LIKE" =~ (debian|ubuntu) ]]
}

# =========================
# RPM
# =========================
install_rpm_repo() {
    echo "Detected RPM-based system (ID=$OS_ID). Setting up libutils repo..."

    $SUDO curl -fsSL -o /etc/yum.repos.d/libutils.repo \
        "$BASE_URL/libutils.repo"

    $SUDO rpm --import "$BASE_URL/RPM-GPG-KEY-tsukini"

    echo "libutils repo installed. You can now run: $SUDO dnf install libutils"
}

# =========================
# DEB
# =========================
install_deb_repo() {
    echo "Detected DEB-based system (ID=$OS_ID). Setting up libutils repo..."

    $SUDO install -m 0755 -d /etc/apt/keyrings

    curl -fsSL "$BASE_URL/RPM-GPG-KEY-tsukini" \
        | gpg --dearmor | $SUDO tee /etc/apt/keyrings/libutils.gpg > /dev/null
    $SUDO chmod a+r /etc/apt/keyrings/libutils.gpg

    ARCH="$(dpkg --print-architecture)"

    echo "deb [arch=${ARCH} signed-by=/etc/apt/keyrings/libutils.gpg] ${BASE_URL}/debian stable main" \
        | $SUDO tee /etc/apt/sources.list.d/libutils.list > /dev/null

    $SUDO apt-get update

    echo "libutils repo installed. You can now run: $SUDO apt install libutils"
}

# =========================
# Dispatch
# =========================
if is_rpm_based; then
    install_rpm_repo
elif is_deb_based; then
    install_deb_repo
else
    echo "Error: unsupported distribution (ID=$OS_ID, ID_LIKE=$OS_ID_LIKE)." >&2
    echo "Supported: Fedora/RHEL-based (rpm) and Debian/Ubuntu-based (deb)." >&2
    exit 1
fi
