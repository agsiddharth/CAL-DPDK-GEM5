FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

## Instal gem5 dependencies
RUN apt -y update && apt -y upgrade && \
    apt -y install build-essential git m4 scons zlib1g zlib1g-dev \
    libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
    python3-dev doxygen libboost-all-dev libhdf5-serial-dev python3-pydot \
    libpng-dev libelf-dev pkg-config pip python3-venv black git-lfs psmisc ssh

## Setup SSH for dist-gem5
RUN ssh-keygen -t rsa -q -N '' -f ~/.ssh/id_rsa && cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys && chmod og-wx ~/.ssh/authorized_keys 

## Install buildroot dependencies
RUN apt -y install sed perl rsync findutils wget libncurses5 libncurses5-dev cpio unzip bc binutils

## install buildroot
RUN mkdir -p /buildroot && wget -qO - https://buildroot.org/downloads/buildroot-2022.02.8.tar.gz | tar -xzvf - -C /buildroot --strip-component 1

## ENV Vars for GEM5/BUILDROOT
ENV GIT_ROOT=/workspaces/CAL-DPDK-GEM5

## For devcontainer, persists bash history
RUN SNIPPET="export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
    && echo "$SNIPPET" >> "/root/.bashrc"