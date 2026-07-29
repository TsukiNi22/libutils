#!/bin/bash
set -euo pipefail

BASE_URL="https://tsukini22.github.io/libutils"

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

    sudo curl -fsSL -o /etc/yum.repos.d/libutils.repo \
        "$BASE_URL/libutils.repo"

    sudo rpm --import "$BASE_URL/RPM-GPG-KEY-tsukini"

    echo "libutils repo installed. You can now run: sudo dnf install libutils"
}

# =========================
# DEB
# =========================
install_deb_repo() {
    echo "Detected DEB-based system (ID=$OS_ID). Setting up libutils repo..."

    sudo install -m 0755 -d /etc/apt/keyrings

    curl -fsSL "$BASE_URL/RPM-GPG-KEY-tsukini" \
        | gpg --dearmor | sudo tee /etc/apt/keyrings/libutils.gpg > /dev/null
    sudo chmod a+r /etc/apt/keyrings/libutils.gpg

    ARCH="$(dpkg --print-architecture)"

    echo "deb [arch=${ARCH} signed-by=/etc/apt/keyrings/libutils.gpg] ${BASE_URL}/debian stable main" \
        | sudo tee /etc/apt/sources.list.d/libutils.list > /dev/null

    sudo apt-get update

    echo "libutils repo installed. You can now run: sudo apt install libutils"
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
