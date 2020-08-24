ARG VARIANT=focal
FROM mcr.microsoft.com/vscode/devcontainers/base:debian

WORKDIR /work

RUN apt-get update \
    && apt-get install -y libncurses5 git wget bzip2 cmake ninja-build

# pull the gcc-arm-none-eabi tarball
RUN wget -qO- https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 | tar -xj

ENV PATH $PATH:/work/gcc-arm-none-eabi-9-2019-q4-major/bin