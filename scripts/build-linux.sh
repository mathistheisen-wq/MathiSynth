#!/usr/bin/env bash
# Linux build (useful for CI / Bitwig / Reaper). Needs: build-essential cmake ninja-build
# libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxext-dev libfreetype-dev libfontconfig1-dev
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/MathiSynthTests_artefacts/Release/MathiSynthTests
