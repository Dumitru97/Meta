include(ExternalProject)

ExternalProject_Add(
  Vc
  GIT_REPOSITORY https://github.com/VcDevel/Vc.git
  GIT_TAG        1.4
  PREFIX         ${CMAKE_SOURCE_DIR}/libs
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/libs/Vc/Vc-src
  BINARY_DIR     ${CMAKE_SOURCE_DIR}/libs/Vc/Vc-build/Vc-build
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_SOURCE_DIR}/libs/Vc
                 -DCMAKE_C_COMPILER=C:/LLVM/bin/clang.exe
                 -DCMAKE_CXX_COMPILER=C:/LLVM/bin/clang++.exe
)