#!/bin/bash
set -e

echo "[Anvil] Stage 1: Compiling Bootstrapper..."
mkdir -p bin

# Compile main.cpp -> bin/anvil
# -I src allows it to find "anvil/api.hpp" inside src/
clang++ -std=c++20 -I src src/main/main.cpp -o bin/anvil

echo "[Anvil] Bootstrapper Ready. Handing over control..."
echo ""

# Run the bootstrapper to build the project defined in build.cpp
./bin/anvil build