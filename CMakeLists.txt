﻿cmake_minimum_required (VERSION 3.19)

project ("meta_SuperBuild")
#set(CMAKE_VERBOSE_MAKEFILE ON)

#Libraries
include(cmake/DlBuildVc.cmake)

#Our Meta optimization library
add_subdirectory ("meta_lib")

#Generate simulation headers for test project
ExternalProject_Add(
    meta_gen_test
    PREFIX          "${CMAKE_BINARY_DIR}/meta_gen_test"
    SOURCE_DIR      "${CMAKE_SOURCE_DIR}/meta_gen_test"
    BINARY_DIR      "${CMAKE_BINARY_DIR}/meta_gen_test"
    INSTALL_COMMAND ""
    DEPENDS Vc
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_CXX_STANDARD=23
               -DCMAKE_GENERATOR_TOOLSET=ClangCL
               -DCMAKE_C_COMPILER=C:/LLVM/bin/clang.exe
               -DCMAKE_CXX_COMPILER=C:/LLVM/bin/clang++.exe
)

#Main, test project
include(ExternalProject)
ExternalProject_Add(
    meta_test
    PREFIX          "${CMAKE_BINARY_DIR}/meta_test"
    SOURCE_DIR      "${CMAKE_SOURCE_DIR}/meta_test"
    BINARY_DIR      "${CMAKE_BINARY_DIR}/meta_test"
    INSTALL_COMMAND ""
    DEPENDS Vc meta_lib meta_gen_test
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_CXX_STANDARD=23
               -DCMAKE_GENERATOR_TOOLSET=ClangCL
               -DCMAKE_C_COMPILER=C:/LLVM/bin/clang.exe
               -DCMAKE_CXX_COMPILER=C:/LLVM/bin/clang++.exe
)


#        "generator": "Visual Studio 17 2022",
#        "toolset": "ClangCL",