# This is a dummy so that IDEs can provide some form of intellisense
# This project does not use CMAKE

cmake_minimum_required(VERSION 3.20)
project(Anvil_IDE_Only)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.hpp")

list(APPEND SRC_FILES "build.cpp")

add_executable(Anvil_Dummy ${SRC_FILES}
        src/anvil/ninja.hpp
        src/anvil/driver.cpp
        src/anvil/script_compiler.hpp
        src/main/clean_command.hpp
        src/main/run_command.hpp
        src/test/test_1.cpp
)