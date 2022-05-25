#!/usr/bin/env bash

set -e
set -x

node src/amalgamate.js
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Profiling -DUNIT_LIBRARY_PROJECT=ON -DCODE_COVERAGE=ON
cmake --build build
cd build && make coverage
