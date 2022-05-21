cmake_minimum_required (VERSION 3.19)

project(meta_test)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_executable(meta_test "main.cpp" "src/MetaTU.cpp")
add_executable(meta_test_precompute "precompute_main.cpp" "precompute_settings.cpp")
set_source_files_properties("precompute_main.cpp" PROPERTIES COMPILE_FLAGS -Wno-unused-local-typedef -Wno-deprecated-enum-enum-conversion -Wno-unused-variable)

#Compiler options
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set_target_properties(meta_test PROPERTIES VS_GLOBAL_LLVMInstallDir "C:/LLVM")
	set_target_properties(meta_test_precompute PROPERTIES VS_GLOBAL_LLVMInstallDir "C:/LLVM")

	set(compiler_flags	"SHELL:-Xclang -std=c++20"
						"SHELL:-Xclang -freflection"
						-mavx2
						"SHELL:-Xclang -fconstexpr-steps"
						"SHELL:-Xclang 1000000000"
						"SHELL:-Xclang -Wno-deprecated-enum-enum-conversion"
	) #-mavx2 -Xclang -O3
else()
	set_target_properties(meta_test PROPERTIES CXX_STANDARD 23)
	set_target_properties(meta_test_precompute PROPERTIES CXX_STANDARD 23)
	set(compiler_flags -std=c++20 -freflection -mavx2 -fconstexpr-steps 1000000000 -Wno-deprecated-enum-enum-conversion) #-mavx2 -Xclang -O3
endif()

target_compile_options(meta_test PRIVATE ${compiler_flags})
target_compile_options(meta_test_precompute PRIVATE ${compiler_flags})

#Linker options
if (CMAKE_GENERATOR MATCHES "Visual Studio")
target_link_options(meta_test_precompute PRIVATE /STACK:"10000000")
else()
target_link_options(meta_test_precompute PRIVATE -Wl,--stack,10000000)
endif()

# Preprocessor definitions
target_compile_definitions(meta_test PRIVATE Vc_GNU_ASM)
target_compile_definitions(meta_test_precompute PRIVATE Vc_GNU_ASM META_PRECOMPUTE_PHASE META_PRECOMPUTE_HEADER_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/include/generated/")

# Linking libraries and includes
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../libs/Vc/Vc-src/cmake)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/../libs/Vc/Vc-build//Vc-build/cmake)

find_package(Vc REQUIRED)

target_link_libraries(meta_test PRIVATE Vc::Vc)
target_link_libraries(meta_test_precompute PRIVATE Vc::Vc)

target_include_directories(meta_test PRIVATE ${CMAKE_SOURCE_DIR}/../meta_lib/include)
target_include_directories(meta_test_precompute PRIVATE ${CMAKE_SOURCE_DIR}/../meta_lib/include)