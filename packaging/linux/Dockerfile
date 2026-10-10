FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        desktop-file-utils \
        file \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libgl1-mesa-dev \
        libpulse-dev \
        libsdl2-dev \
        libswscale-dev \
        libwayland-dev \
        libx11-dev \
        libxcursor-dev \
        libxi-dev \
        libxinerama-dev \
        libxkbcommon-dev \
        libxrandr-dev \
        imagemagick \
        ninja-build \
        patchelf \
        pkg-config \
        zsync \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work

COPY build-portable.sh /usr/local/bin/build-projectcane-portable
RUN chmod +x /usr/local/bin/build-projectcane-portable

ENTRYPOINT ["/usr/local/bin/build-projectcane-portable"]
