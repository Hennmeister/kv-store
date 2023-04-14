#!/bin/bash

cmake -Bbuild -DCMAKE_BUILD_TYPE=Release && make -C build && ./build/kv-store-test
