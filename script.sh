#!/bin/bash

sudo curl -o /etc/yum.repos.d/libutils.repo \
https://tsukini22.github.io/utils/libutils.repo

sudo rpm --import \
https://tsukini22.github.io/utils/RPM-GPG-KEY-tsukini
