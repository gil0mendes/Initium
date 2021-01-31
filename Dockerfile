FROM debian AS base

ENV PKGDIR /pkg
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo

# Install global tools
RUN apt-get update && apt-get install -y \
    curl \
    file \
    make \
    python3 \
    && rm -rf /var/lib/apt/lists/*

FROM base AS build
ENV BINUTILS_VERSION=2.36 \
    PARALLEL_MAKE=-j4

# Install needed dependencies to build binutils
RUN apt-get update && apt-get install -y \
    bzip2 \
    file \
    gcc \
    musl-dev

# Download an unpack binutils source code
WORKDIR /src
RUN set -x \
    && curl -fsSLO --compressed "http://ftpmirror.gnu.org/binutils/binutils-$BINUTILS_VERSION.tar.bz2" \
    && tar -xjf binutils-$BINUTILS_VERSION.tar.bz2

# Build binutils
WORKDIR /build
RUN /src/binutils-$BINUTILS_VERSION/configure --silent \
    --disable-multilib \
    # Disables internationalization as i18n is not needed for the cross-compile tools
    --disable-nls \
    # Adds 64 bit support and UEFI
    --enable-64-bit-bfd \
    --enable-targets=i386-efi-pe,x86_64-efi-pe \
    && make --silent $PARALLEL_MAKE \
    && make --silent DESTDIR="$PKGDIR" install

FROM base
ENV PATH=/usr/local/cargo/bin:$PATH

COPY --from=build "$PKGDIR"/usr/local /usr/local

# Install Rust
RUN set -eux; \
    curl -fsSLO --compressed "https://static.rust-lang.org/rustup/dist/x86_64-unknown-linux-gnu/rustup-init"; \
    chmod +x rustup-init; \
    ./rustup-init -y --no-modify-path --default-toolchain nightly; \
    rm rustup-init; \
    chmod -R a+w $RUSTUP_HOME $CARGO_HOME; \
    rustup component add rust-src; \
    rustup --version; \
    cargo --version; \ 
    rustc --version;

WORKDIR /code
CMD ["/bin/bash"]
