# Ubuntu 24.04 gives GCC 13 (full C++17/20) plus a working arm64 Valgrind.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      g++ gdb cmake make pkg-config \
      valgrind \
      libgtest-dev googletest \
      linux-tools-common \
      iproute2 iputils-ping net-tools tcpdump \
      python3 ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
CMD ["/bin/bash"]
