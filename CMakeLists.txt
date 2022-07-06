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

option(SET_RPATH "Set runtime path to the ${CMAKE_INSTALL_PREFIX}/lib." ON)
option(SWFLIB_STATIC "Link swflib statically instead of dynamic linking." ON)
if(SWFLIB_STATIC)
  find_package(ZLIB REQUIRED)
  find_package(LibLZMA REQUIRED)
endif()
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

if(SWFLIB_STATIC)
  target_link_libraries(unpacker PRIVATE swflib::swflib-static)
else()
  target_link_libraries(unpacker PRIVATE swflib::swflib)
  if(SET_RPATH)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif()
endif()

# Install
install(TARGETS unpacker RUNTIME DESTINATION bin COMPONENT bin)
