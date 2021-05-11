#!/bin/bash

export ARNOLD_ROOT=~/arnold/SDK/6.2.0.1
export HFS=/opt/hfs18.5.499

export NAME="closest"

pushd $HFS
source houdini_setup
popd

#export HCUSTOM_LDFLAGS="-Wl,-R${HFS}/dsolib"
hcustom -e -i ./build src/$NAME.cpp -L ${ARNOLD_ROOT}/bin -lai -I ${ARNOLD_ROOT}/include -L ${HFS}/dsolib -lHoudiniGEO -lHoudiniUT -ljemalloc

