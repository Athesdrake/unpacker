cmake_minimum_required(VERSION 3.14)
project(unpacker CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(unpacker)
target_include_directories(unpacker PUBLIC include)
add_subdirectory(src)

# Find dependencies
find_package(cpr CONFIG REQUIRED)
# find_package(argparse CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(swflib CONFIG REQUIRED)

# Argparse's version is broken on vcpkg
include(FetchContent)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v2.4)

FetchContent_GetProperties(argparse)
if(NOT argparse_POPULATED)
  FetchContent_Populate(argparse)
  add_subdirectory(${argparse_SOURCE_DIR} ${argparse_BINARY_DIR}
                   EXCLUDE_FROM_ALL)
endif()

target_link_libraries(unpacker PRIVATE argparse::argparse)
target_link_libraries(unpacker PRIVATE cpr::cpr)
target_link_libraries(unpacker PRIVATE fmt::fmt-header-only)
target_link_libraries(unpacker PRIVATE swflib::swflib)
