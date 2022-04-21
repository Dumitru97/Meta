cmake_minimum_required (VERSION 3.19)

set(header_files "meta.h"
                 "1_Orders.h"
                 "2_Parameters.h"
                 "3_Functions.h"
                 "4_Arguments.h"
                 "5_Calls.h"
                 "optimizer/CostFunctions.h"
                 "optimizer/SimulatedAnnealing.h"
                 "optimizer/SimulatedAnnealing_Implementation1.h"
                 "misc/meta_utils.h"
                 "misc/print.h"
                 "misc/func_format.h")
list(TRANSFORM header_files PREPEND "include/")

add_library (meta_lib INTERFACE ${header_files})
target_include_directories(meta_lib INTERFACE "include/")