project(
    'unpacker',
    'cpp',
    default_options: ['cpp_std=c++17'],
    version: '0.2.1',
)

swflib = dependency('swflib')
argparse = dependency('argparse')
cpr = dependency('cpr')
fmt = dependency('fmt')

incdir = include_directories('include')
unpack = library(
    'unpack',
    'lib/unpacker.cpp',
    'lib/string_finder.cpp',
    include_directories: incdir,
    dependencies: [swflib, cpr],
)
unpack_dep = declare_dependency(include_directories: incdir, link_with: unpack)

executable(
    'unpacker',
    'src/main.cpp',
    'src/utils.cpp',
    include_directories: incdir,
    dependencies: [swflib, argparse, fmt],
    link_with: unpack,
    install: true,
)