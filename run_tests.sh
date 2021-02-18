#!/bin/bash

set -x
set -e

mkdir -p build
cd build
cmake -DCMDLINEFLAGS_TESTS=ON ..

make clean
make

lcov --base-directory . --directory CMakeFiles --initial --capture --output-file coverage.init

make test

lcov --rc lcov_branch_coverage=1 --directory CMakeFiles --capture  --output-file coverage.run
lcov --rc lcov_branch_coverage=1 --add-tracefile coverage.init --add-tracefile coverage.run --output-file coverage.total
lcov --rc lcov_branch_coverage=1 --remove coverage.total /usr/include/x86_64-linux-gnu/bits/stdio2.h --output-file coverage.info

genhtml --rc lcov_branch_coverage=1 coverage.info --output-directory lcov.d
rm coverage.*
