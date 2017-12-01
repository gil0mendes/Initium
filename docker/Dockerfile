FROM rustlang/rust:nightly
LABEL maintainer="Gil Mendes <gil00mendes@gmail.com>"

# Override image name of base image
ENV IMAGE_NAME=initium-loader-build

# =============================================================================
# Install additional toolchain for building the OS
# =============================================================================
RUN set -ex;                                       \
    apt-get update;                                \
    apt-get install -q -y --no-install-recommends  \
        nasm                                       \
        binutils                                   \
        gosu                                       \
        sudo                                       \
        grub-common                                \
        xorriso                                    \
        grub-pc-bin                                \
        parted                                     \
        mtools                                     \
        udev                                       \
        ;                                          \
    cargo install xargo;                           \
    cargo install cargo-config;                    \
    rustup component add rust-src;                 \
    apt-get autoremove -q -y;                      \
    apt-get clean -q -y;                           \
    rm -rf /var/lib/apt/lists/*

# TODO: install lld

COPY entrypoint.sh /usr/local/bin/
COPY .bash_aliases /etc/skel/

ENTRYPOINT ["bash", "/usr/local/bin/entrypoint.sh"]
CMD ["/bin/bash"]
