cmake_minimum_required(VERSION 3.20)
project(Anvil_IDE_Only)

set(CMAKE_CXX_STANDARD 20)

# Include the root 'src' so CLion resolves "anvil/api.hpp"
include_directories(src)

# Add all files so they appear in the project view
file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.hpp")

# CRITICAL: Treat build.cpp as C++ source
list(APPEND SRC_FILES "build.cpp")

add_executable(Anvil_Dummy ${SRC_FILES}
        src/anvil/ninja.hpp
        src/anvil/driver.cpp
        src/anvil/script_compiler.hpp
        src/main/clean_command.hpp
)