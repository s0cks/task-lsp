ARG UBUNTU_VERSION=26.04
FROM ubuntu:${UBUNTU_VERSION} AS builder
ARG GO_VERSION=1.26.4
ARG MESON_VERSION=1.11.1

ENV DEBIAN_FRONTEND=noninteractive
ENV GO_VERSION=${GO_VERSION}
ENV MESON_VERSION=${MESON_VERSION}
ENV CURL_VERSION=8.18.0-1ubuntu2.5
ENV PIP3_VERSION=25.1.1+dfsg-1ubuntu2
ENV PY3_VERSION=3.14.3-0ubuntu2
ENV NINJA_VERSION=1.13.2-1
ENV BUILD_ESSENTIAL_VERSION=12.12ubuntu2.26.04.2
ENV PKG_CONFIG_VERSION=2.5.1-4
ENV CA_CERTIFICATES_VERSION=20260601~26.04.1
ENV GIT_VERSION=1:2.53.0-1ubuntu1
ENV CMAKE_VERSION=4.2.3-2ubuntu2
ENV GTEST_VERSION=1.17.0-1build1

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    curl=${CURL_VERSION} \
    ninja-build=${NINJA_VERSION} \
    python3=${PY3_VERSION} \
    python3-pip=${PIP3_VERSION} \
    build-essential=${BUILD_ESSENTIAL_VERSION} \
    pkg-config=${PKG_CONFIG_VERSION} \
    ca-certificates=${CA_CERTIFICATES_VERSION} \
    git=${GIT_VERSION} \
    cmake=${CMAKE_VERSION} \
    libgtest-dev=${GTEST_VERSION} \
 && rm -rf /var/lib/apt/lists/* \
 && pip3 install \
      --no-cache-dir \
      --break-system-packages \
      meson==${MESON_VERSION}

# ╭────────────╮
# │ Install Go │
# ╰────────────╯
RUN curl -sSL "https://golang.org/dl/go${GO_VERSION}.linux-amd64.tar.gz" -o go-${GO_VERSION}.tar.gz \
 && tar -C /usr/local -xzf go-${GO_VERSION}.tar.gz
ENV GOPATH=/go
ENV PATH=$PATH:/usr/local/go/bin:$GOPATH/bin
# ──────────────────────────────────────────────────────────────────────

RUN mkdir -p /workspace
WORKDIR /workspace

#TODO(@s0cks): install vscode/ build deps
