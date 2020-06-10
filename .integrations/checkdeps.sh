#!/usr/bin/env bash

set -ev

if which xclip; then
    echo "All dependencies found."
    exit 0;
else 
    echo "Missing xclip!"
    exit 1;
fi
