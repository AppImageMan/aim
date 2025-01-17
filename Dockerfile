FROM debian:bookworm

# Install necessary packages
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        appstream \
        build-essential \
        ca-certificates \
        cmake \
        clang-format \
        clang-tidy \
        curl \
        file \
        gdb \
        git \
        gpg \
        kmod \
        sudo \
        unzip \
        vim \
        wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Add a non-root user for building
RUN useradd -G sudo -m dev
RUN mkdir /home/dev/src && chown -R dev:dev /home/dev/src

# Install the VS Code Debug Adapter Protocol (DAP) debugger if needed
RUN mkdir -p /home/dev/.vscode-server/extensions \
    && mkdir -p /home/dev/.vscode-server-insiders/extensions

# Download AppImage Tool
RUN wget https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
RUN chmod +x appimagetool-x86_64.AppImage

WORKDIR /home/dev/src

#COPY . /home/dev/src
USER dev
