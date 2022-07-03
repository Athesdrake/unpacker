# Unpacker
A Transformice tool that let you unpack the SWF easily.

## Usage
The most basic usage:
```sh
unpacker unpacked.swf
```
It will download the SWF from the server and unpack it  to the specified file.
You can specify the file to unpack using the `-i` argument.
```sh
unpacker -i Transformice.swf unpacked.swf
```

It also possible to use stdin as the input and stdout as the output by defining `-i` to `-` and the output filename to `-` respectively.

Using stdin as the input:
```sh
unpacker -i - unpacked.swf
```

Using stdout as the output:
```sh
unpacker -
```

## Building from source
Few libraries are needed in order to this project to compile.
 - [argparse](https://github.com/p-ranav/argparse)
 - [fmt](https://github.com/fmtlib/fmt)
 - [cpr](https://github.com/libcpr/cpr)
 - [swflib](https://github.com/Athesdrake/swflib)

You can install most `fmt` and `cpr` using [`vcpkg`](https://vcpkg.io/en/index.html).
`argparse` is downloaded using CMake's `FetchContent` utility.

As for `swflib`, you'll need to get it from [here](https://github.com/Athesdrake/swflib).

Install the dependencies and configure the CMake project. You'll need to provide the toolchain file to cmake in order to find the libraries.
```sh
mkdir build
cd build
cmake .. "-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake"
```
You can also provide additional directives, such as `-Dswflib_DIR=[path to swflib]` to help cmake finding swflib or `-DCMAKE_BUILD_TYPE=Release` to build in release mode.

Afterward, you'll be able to build the project:
```sh
cmake --build .
```
