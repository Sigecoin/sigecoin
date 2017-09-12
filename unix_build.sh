#!/bin/sh

mkdir .build
cd ./.build
set params = -DCMAKE_BUILD_TYPE=RELEASE
cmake -G "Unix Makefiles" $params ../
