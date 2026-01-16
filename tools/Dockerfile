# ────────────────────────────────────────────────
#  minishell – dev container with Criterion
# ────────────────────────────────────────────────
FROM ubuntu:22.04

# --- base toolchain ----------------------------------------------------------
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential clang lldb valgrind gdb gdbserver \
        git curl ca-certificates \
        meson ninja-build cmake pkg-config \
        libssl-dev lsof libreadline-dev  # Criterion needs this for its tests
# ---------------------------------------------------------------------------

# --- build & install Criterion ----------------------------------------------
ARG CRITERION_TAG=v2.4.2
RUN git clone --depth 1 --branch ${CRITERION_TAG} \
            https://github.com/Snaipe/Criterion.git /tmp/criterion \
     && cd /tmp/criterion \
     && meson setup build \
     && ninja -C build   \
     && ninja -C build install \
     && ldconfig \
     && rm -rf /tmp/criterion
# ---------------------------------------------------------------------------

# --- nice coloured prompt ----------------------------------------------------
RUN echo 'export PS1="\[\e[38;5;42m\]\u@\h:\w$\[\e[0m\] "' >> /etc/bash.bashrc
# ---------------------------------------------------------------------------


# --- user conveniences ------------------------------------------------------
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

ARG UID=1000
ARG GID=1000
RUN groupadd -g ${GID} dev && useradd -m -u ${UID} -g ${GID} -s /bin/bash dev

WORKDIR /workspace
CMD ["bash"]
