#!/bin/bash

sudo curl -o /etc/yum.repos.d/libutils.repo \
https://tsukini22.github.io/libutils/libutils.repo

sudo rpm --import \
https://tsukini22.github.io/libutils/RPM-GPG-KEY-tsukini
