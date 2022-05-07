cmake_minimum_required (VERSION 3.19)

project(meta_gen_test)

set(header_files "Actors.h"
				 "DefaultCallOrder.h"
				 "Includes.h"
				 "Objects.h"
				 "Orders.h"
				 "Shared.h"
			 	 "Source.cpp"
			 	 "StableHeaders.h"
				 "Variables.h"
)
list(TRANSFORM header_files PREPEND "src/")

add_executable (meta_gen_test ${header_files})

set_target_properties(meta_gen_test PROPERTIES CXX_STANDARD 23)
target_compile_definitions(meta_gen_test PRIVATE OUTPUT_DIRECTORY_NO_SLASH="${CMAKE_CURRENT_SOURCE_DIR}/../meta_test/include/simulation" CONSOLE_INPUT)