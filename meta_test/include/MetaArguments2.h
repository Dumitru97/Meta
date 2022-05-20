#pragma once
#include "meta.h"
#include "simulation/TestIncludes.h"

extern Meta::SAFunctionOrder::SAParams sa_params;
META_DEFINITIONS_ON_FN_OP(ON, FN, BBFunctionOrderOP)
META_PRECOMPUTE_OR_CREATE_ARGUMENTS(ON, FN, BBFunctionOrderOP, sa_params)
#include META_INCLUDE_IDXS_HEADER(generated, ON, FN, BBFunctionOrderOP, sa_params)