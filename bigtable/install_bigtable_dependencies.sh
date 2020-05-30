#!/bin/bash
# ---------------------------------------------------------------------------- #
# Init
# Make sure only root can run our script. Run with SUDO.
# ---------------------------------------------------------------------------- #
echo "Make sure that the Google Bigtable Database is running. This script will take ~30 min."
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

# ---------------------------------------------------------------------------- #
# Install Google Bigtable Command Line Tool
# ---------------------------------------------------------------------------- #
# gcloud components update
apt-get install google-cloud-sdk-cbt

# ---------------------------------------------------------------------------- #
# Configure Google Bigtable Credentials
# ---------------------------------------------------------------------------- #
echo project = mrjk-273504 > ~/.cbtrc
echo instance = mrjk-database >> ~/.cbtrc

# ---------------------------------------------------------------------------- #
# Add Blog Post data to Google Bigtable
# ---------------------------------------------------------------------------- #
./bigtable.sh

# ---------------------------------------------------------------------------- #
# Install Google Bigtable Libraries and Dependencies
# There is no apt-get install google-cloud-cpp :(
# https://github.com/googleapis/google-cloud-cpp/blob/master/doc/packaging.md
# ---------------------------------------------------------------------------- #
# Install the minimal development tools, OpenSSL and libcurl:
apt-get update && apt-get --no-install-recommends install -y apt-transport-https apt-utils \
        automake build-essential ccache cmake ca-certificates git gcc g++ \
        libcurl4-openssl-dev libssl-dev libtool m4 make \
        pkg-config tar wget zlib1g-dev

mkdir $HOME/Downloads

# Installs Abseil
cd $HOME/Downloads
wget -q https://github.com/abseil/abseil-cpp/archive/20200225.2.tar.gz && \
    tar -xf 20200225.2.tar.gz && \
    cd abseil-cpp-20200225.2 && \
    sed -i 's/^#define ABSL_OPTION_USE_\(.*\) 2/#define ABSL_OPTION_USE_\1 0/' "absl/base/options.h" && \
    cmake \
      -DCMAKE_BUILD_TYPE="Release" \
      -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=yes \
      -H. -Bcmake-out/abseil && \
cmake --build cmake-out/abseil --target install -- -j ${NCPU} && \
ldconfig && cd /var/tmp && rm -fr build

# Installs Protobuf
cd $HOME/Downloads
wget -q https://github.com/google/protobuf/archive/v3.11.3.tar.gz && \
    tar -xf v3.11.3.tar.gz && \
    cd protobuf-3.11.3/cmake && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -Dprotobuf_BUILD_TESTS=OFF \
        -H. -Bcmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
ldconfig

# Installs c-ares
cd $HOME/Downloads
wget -q https://github.com/c-ares/c-ares/archive/cares-1_14_0.tar.gz && \
    tar -xf cares-1_14_0.tar.gz && \
    cd c-ares-cares-1_14_0 && \
    ./buildconf && ./configure && make -j ${NCPU:-4} && \
make install && \
ldconfig

# Installs G-RPC
cd $HOME/Downloads
wget -q https://github.com/grpc/grpc/archive/v1.29.1.tar.gz && \
    tar -xf v1.29.1.tar.gz && \
    cd grpc-1.29.1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DgRPC_INSTALL=ON \
        -DgRPC_BUILD_TESTS=OFF \
        -DgRPC_ABSL_PROVIDER=package \
        -DgRPC_CARES_PROVIDER=package \
        -DgRPC_PROTOBUF_PROVIDER=package \
        -DgRPC_SSL_PROVIDER=package \
        -DgRPC_ZLIB_PROVIDER=package \
        -H. -Bcmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
ldconfig

# Installs crc32c
cd $HOME/Downloads
wget -q https://github.com/google/crc32c/archive/1.1.0.tar.gz && \
    tar -xf 1.1.0.tar.gz && \
    cd crc32c-1.1.0 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCRC32C_BUILD_TESTS=OFF \
        -DCRC32C_BUILD_BENCHMARKS=OFF \
        -DCRC32C_USE_GLOG=OFF \
        -H. -Bcmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
ldconfig

# Installs google-cloud-cpp for Google Bigtable
git clone https://github.com/googleapis/google-cloud-cpp.git $HOME/google-cloud-cpp
cd $HOME/google-cloud-cpp
cmake -DBUILD_TESTING=OFF -H. -Bcmake-out
cmake --build cmake-out -- -j "${NCPU:-4}"
cmake --build cmake-out --target install

# ---------------------------------------------------------------------------- #
# Authentication to the mrjk-web-server to connect to Google Bigtable
# ---------------------------------------------------------------------------- #
cd /usr/src/projects/mrjk-web-server
gcloud iam service-accounts create mrjk-key
gcloud projects add-iam-policy-binding mrjk-273504 --member "serviceAccount:mrjk-key@mrjk-273504.iam.gserviceaccount.com" --role "roles/owner"
gcloud iam service-accounts keys create mrjk-key.json --iam-account mrjk-key@mrjk-273504.iam.gserviceaccount.com
export GOOGLE_APPLICATION_CREDENTIALS="/usr/src/projects/mrjk-web-server/mrjk-key.json"
