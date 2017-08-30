#!/usr/bin/env bash

function travis_abort() {
    echo "Unable to build $1"
    exit 1
}

function before_install() {
    if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        brew update
        brew install uriparser json-c ossp-uuid msgpack
    fi

    if [[ "$COVERITY_SCAN_BRANCH" != 1 ]]; then
        echo -n | openssl s_client -connect scan.coverity.com:443 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' | sudo tee -a /etc/ssl/certs/ca-
    fi
}

function before_script() {

    # The following dependencies are unavailable on apt repositories
    echo "Installing gru"
    wget https://github.com/orpiske/gru/tarball/master -O gru-head.tar.gz
    mkdir gru && tar -xvf gru-head.tar.gz -C gru --strip-components=1
    pushd gru && mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=DEBUG .. || travis_abort gru
    make  || travis_abort gru
    sudo make install  || travis_abort gru
    popd

    echo "Installing bmic"
    wget https://github.com/orpiske/bmic/tarball/master -O bmic-head.tar.gz
    mkdir bmic && tar -xvf bmic-head.tar.gz -C bmic --strip-components=1
    pushd bmic && mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=DEBUG ..  || travis_abort bmic
    make  || travis_abort bmic
    sudo make install  || travis_abort bmic
    popd

    echo "Installing litestomp"
    wget https://github.com/orpiske/litestomp/tarball/master -O litestomp-head.tar.gz
    mkdir litestomp && tar -xvf litestomp-head.tar.gz -C litestomp --strip-components=1
    pushd litestomp && mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=DEBUG ..  || travis_abort litestomp
    make  || travis_abort litestomp
    sudo make install  || travis_abort litestomp
    popd

    echo "Installing Paho MQTT C"
    wget https://github.com/eclipse/paho.mqtt.c/tarball/master -O pahoc-head.tar.gz
    mkdir pahoc && tar -xvf pahoc-head.tar.gz -C pahoc --strip-components=1
    pushd pahoc && mkdir build.paho && cd build.paho
    cmake -DPAHO_WITH_SSL=FALSE -DPAHO_BUILD_SAMPLES=FALSE -DPAHO_BUILD_DOCUMENTATION=FALSE -DCMAKE_BUILD_TYPE=RELEASE ..  || travis_abort paho
    make || travis_abort paho
    sudo make install || travis_abort paho
    popd

    echo "Installing Hdr Histogram C"
    wget https://github.com/HdrHistogram/HdrHistogram_c/tarball/master -O hdr-histogram-c-head.tar.gz
    mkdir hdr-histogram-c && tar -xvf hdr-histogram-c-head.tar.gz -C hdr-histogram-c --strip-components=1
    pushd hdr-histogram-c && mkdir build && cd build && cmake -DCMAKE_USER_C_FLAGS="-fPIC" .. && make && sudo make install ; popd


    if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
        # The version of msgpack on TravisCI is ancient and won't work with this project
        echo "Installing MsgPack C"
        wget https://github.com/msgpack/msgpack-c/tarball/master -O msgpack-c-head.tar.gz
        mkdir msgpack-c && tar -xvf msgpack-c-head.tar.gz -C msgpack-c --strip-components=1
        pushd msgpack-c && mkdir build && cd build
        cmake -DCMAKE_USER_C_FLAGS="-fPIC" .. || travis_abort msgpack
        make || travis_abort msgpack
        sudo make install || travis_abort msgpack
        popd
    fi
}

function build() {
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=DEBUG -DSTOMP_SUPPORT=OFF -DAMQP_SUPPORT=OFF .. || travis_abort msg-perf-tool
    make all || travis_abort msg-perf-tool
    cd ..
}


if [[ -z "$1" ]] ; then
    usage
    exit 1
fi


case "$1" in
	before_install)
		before_install
		;;
	before_script)
		before_script
		;;
	build)
		build
		;;
	*)
		echo $"Usage: $0 {before_install|before_script|build}"
		RETVAL=2
esac




