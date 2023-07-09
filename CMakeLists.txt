cmake_minimum_required(VERSION 3.14)

option(SET_RPATH "Set runtime path to the ${CMAKE_INSTALL_PREFIX}/lib." ON)

project(unpacker CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(unpack SHARED "")
add_library(athes::unpack ALIAS unpack)
target_include_directories(
  unpack PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>)

add_executable(unpacker)
add_subdirectory(lib)
add_subdirectory(src)

# Find dependencies
find_package(cpr CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(swflib CONFIG REQUIRED)

target_link_libraries(unpack PRIVATE cpr::cpr)
target_link_libraries(unpack PUBLIC swflib::swflib)
if(SET_RPATH)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

target_link_libraries(unpacker PRIVATE athes::unpack fmt::fmt-header-only)

# Install stuff
export(
  TARGETS unpack
  NAMESPACE athes::
  FILE ${PROJECT_BINARY_DIR}/unpackTargets.cmake)
export(PACKAGE athes_unpack)

file(RELATIVE_PATH REL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/lib/CMake/unpack
     ${CMAKE_INSTALL_PREFIX}/include)
set(unpack_INCLUDE_DIRS ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
configure_file(cmake/unpackConfig.cmake.in
               ${PROJECT_BINARY_DIR}/athes_unpackConfig.cmake @ONLY)
set(unpack_INCLUDE_DIRS "\${UNPACK_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(
  cmake/unpackConfig.cmake.in
  ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/athes_unpackConfig.cmake @ONLY)

install(
  TARGETS unpack
  EXPORT unpackTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib COMPONENT shlib
  RUNTIME DESTINATION bin COMPONENT bin
  PUBLIC_HEADER DESTINATION include/unpack COMPONENT dev)
install(
  FILES ${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/athes_unpackConfig.cmake
  DESTINATION lib/CMake/unpack
  COMPONENT dev)
install(
  EXPORT unpackTargets
  NAMESPACE athes::
  DESTINATION lib/CMake/unpack
  COMPONENT dev)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
