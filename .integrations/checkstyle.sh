#!/usr/bin/env bash

set -ev

for header in ./*.h; do
    echo $header
    clang-format $header | diff - $header
done

for source in ./*.cc; do
    echo $source
    clang-format $source | diff - $source
done
