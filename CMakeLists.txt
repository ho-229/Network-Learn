# Network-Learn

cmake_minimum_required(VERSION 3.5)

project(Network-Learn)

set(CXX_FLAGS
   -j4
#   -fsanitize=address
#   -fsanitize=undefined
#   -fsanitize=leak
#   -fsanitize-recover=all
#   -fno-omit-frame-pointer
#   -fno-stack-protector
#   -fsanitize=leak
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Subprojects
add_subdirectory(TinyWebServer)
