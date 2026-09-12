ARG UBUNTU_VERSION=26.04
FROM ubuntu:${UBUNTU_VERSION} AS builder
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

ARG GO_VERSION=1.26.4
ARG MESON_VERSION=1.11.1
ARG CURL_VERSION=8.18.0-1ubuntu2.5
ARG PIP3_VERSION=25.1.1+dfsg-1ubuntu2
ARG PY3_VERSION=3.14.3-0ubuntu2
ARG NINJA_VERSION=1.13.2-1
ARG BUILD_ESSENTIAL_VERSION=12.12ubuntu2.26.04.2
ARG PKG_CONFIG_VERSION=2.5.1-4
ARG CA_CERTIFICATES_VERSION=20260601~26.04.1
ARG GIT_VERSION=1:2.53.0-1ubuntu1
ARG CMAKE_VERSION=4.2.3-2ubuntu2
ARG GTEST_VERSION=1.17.0-1build1
ARG RUMDL_VERSION=0.2.73
ARG GOLANGCI_LINT_VERSION=2.13.2
ARG YAMLLINT_VERSION=1.37.1-1

ENV DEBIAN_FRONTEND=noninteractive
ENV GO_VERSION=${GO_VERSION}
ENV MESON_VERSION=${MESON_VERSION}
ENV CURL_VERSION=${CURL_VERSION}
ENV PIP3_VERSION=${PIP3_VERSION}
ENV PY3_VERSION=${PY3_VERSION}
ENV NINJA_VERSION=${NINJA_VERSION}
ENV BUILD_ESSENTIAL_VERSION=${BUILD_ESSENTIAL_VERSION}
ENV PKG_CONFIG_VERSION=${PKG_CONFIG_VERSION}
ENV CA_CERTIFICATES_VERSION=${CA_CERTIFICATES_VERSION}
ENV GIT_VERSION=${GIT_VERSION}
ENV CMAKE_VERSION=${CMAKE_VERSION}
ENV GTEST_VERSION=${GTEST_VERSION}
ENV RUMDL_VERSION=${RUMDL_VERSION}
ENV GOLANGCI_LINT_VERSION=${GOLANGCI_LINT_VERSION}
ENV YAMLLINT_VERSION=${YAMLLINT_VERSION}

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
    yamllint=${YAMLLINT_VERSION} \
 && rm -rf /var/lib/apt/lists/* \
 && pip3 install \
      --no-cache-dir \
      --break-system-packages \
      meson==${MESON_VERSION}

# ╭────────────╮
# │ Install Go │
# ╰────────────╯
ENV GO_DOWNLOAD_URL=https://golang.org/dl/go${GO_VERSION}.linux-amd64.tar.gz
RUN curl -LsSf "${GO_DOWNLOAD_URL}" \
      -o go-${GO_VERSION}.tar.gz \
 && tar \
      -C /usr/local \
      -xzf go-${GO_VERSION}.tar.gz
ENV GOPATH=/go
ENV PATH=$PATH:/usr/local/go/bin:$GOPATH/bin
# ──────────────────────────────────────────────────────────────────────

# ╭───────────────╮
# │ Install rumdl │
# ╰───────────────╯
ENV RUMDL_DOWNLOAD_URL=https://github.com/rvben/rumdl/releases/download/v${RUMDL_VERSION}/rumdl-v${RUMDL_VERSION}-x86_64-unknown-linux-gnu.tar.gz
RUN curl -LsSf ${RUMDL_DOWNLOAD_URL} | tar xzf - -C /usr/local/bin
# ──────────────────────────────────────────────────────────────────────

# ╭───────────────────────╮
# │ Install golangci-lint │
# ╰───────────────────────╯
ENV GOLANGCI_LINT_DOWNLOAD_URL=https://github.com/golangci/golangci-lint/releases/download/v${GOLANGCI_LINT_VERSION}/golangci-lint-${GOLANGCI_LINT_VERSION}-linux-amd64.tar.gz
RUN curl -LsSf "${GOLANGCI_LINT_DOWNLOAD_URL}" \
      -o golangci-lint-${GOLANGCI_LINT_VERSION}-linux-amd64.tar.gz \
 && tar xf golangci-lint-${GOLANGCI_LINT_VERSION}-linux-amd64.tar.gz \
 && mv golangci-lint-${GOLANGCI_LINT_VERSION}-linux-amd64/golangci-lint /usr/local/bin
# ──────────────────────────────────────────────────────────────────────

RUN mkdir -p /workspace
WORKDIR /workspace
