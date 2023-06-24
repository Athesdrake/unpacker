cmake_minimum_required(VERSION 3.14)

option(SET_RPATH "Set runtime path to the ${CMAKE_INSTALL_PREFIX}/lib." ON)
option(SWFLIB_STATIC "Link swflib statically instead of dynamic linking." ON)
if(SWFLIB_STATIC)
  list(APPEND VCPKG_MANIFEST_FEATURES "static")
endif()

project(unpacker CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(tfm.unpack SHARED "")
add_library(athes::tfm::unpack ALIAS tfm.unpack)
target_include_directories(tfm.unpack PUBLIC include)

add_executable(unpacker)
add_subdirectory(lib)
add_subdirectory(src)

# Find dependencies
find_package(cpr CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

if(SWFLIB_STATIC)
  find_package(ZLIB REQUIRED)
  find_package(LibLZMA REQUIRED)
endif()
find_package(swflib CONFIG REQUIRED)

target_link_libraries(tfm.unpack PRIVATE cpr::cpr)

if(SWFLIB_STATIC)
  target_link_libraries(tfm.unpack PUBLIC swflib::swflib-static)
else()
  target_link_libraries(tfm.unpack PUBLIC swflib::swflib)
  if(SET_RPATH)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif()
endif()

target_link_libraries(unpacker PRIVATE tfm.unpack fmt::fmt-header-only)

# Install
install(TARGETS tfm.unpack RUNTIME DESTINATION lib COMPONENT lib)
