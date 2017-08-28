#!/usr/bin/env bash

function before_install() {
    if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        brew update
        brew install uriparser json-c ossp-uuid msgpack
    fi
}

function before_script() {
    echo "Installing gru"

    wget https://github.com/orpiske/gru/tarball/master -O gru-head.tar.gz
    mkdir gru && tar -xvf gru-head.tar.gz -C gru --strip-components=1
    pushd gru && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=DEBUG .. && make && sudo make install ; popd

    echo "Installing bmic"
    wget https://github.com/orpiske/bmic/tarball/master -O bmic-head.tar.gz
    mkdir bmic && tar -xvf bmic-head.tar.gz -C bmic --strip-components=1
    pushd bmic && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=DEBUG .. && make && sudo make install ; popd

    echo "Installing litestomp"
    wget https://github.com/orpiske/litestomp/tarball/master -O litestomp-head.tar.gz
    mkdir litestomp && tar -xvf litestomp-head.tar.gz -C litestomp --strip-components=1
    pushd litestomp && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=DEBUG .. && make && sudo make install ; popd

    echo "Installing Paho MQTT C"
    wget https://github.com/eclipse/paho.mqtt.c/tarball/master -O pahoc-head.tar.gz
    mkdir pahoc && tar -xvf pahoc-head.tar.gz -C pahoc --strip-components=1
    pushd pahoc && mkdir build.paho && cd build.paho && cmake -DPAHO_WITH_SSL=FALSE -DPAHO_BUILD_SAMPLES=FALSE -DPAHO_BUILD_DOCUMENTATION=FALSE -DCMAKE_BUILD_TYPE=RELEASE .. && make && sudo make install ; popd


    echo "Installing Hdr Histogram C"
    wget https://github.com/HdrHistogram/HdrHistogram_c/tarball/master -O hdr-histogram-c-head.tar.gz
    mkdir hdr-histogram-c && tar -xvf hdr-histogram-c-head.tar.gz -C hdr-histogram-c --strip-components=1
    pushd hdr-histogram-c && mkdir build && cd build && cmake -DCMAKE_USER_C_FLAGS="-fPIC" .. && make && sudo make install ; popd
}

function build() {
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=DEBUG -DSTOMP_SUPPORT=OFF -DAMQP_SUPPORT=OFF ..
    make all
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




