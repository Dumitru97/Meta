#pragma once
#include "meta.h"
#include "simulation/TestIncludes.h"

extern Meta::BBFunctionOrder::BBParams bb_params;
META_DEFINITIONS_ON_FN_OP(ON, FN, BBFunctionOrderOP)
META_PRECOMPUTE(ON, FN, BBFunctionOrderOP, bb_params)
#include META_INCLUDE_IDXS_HEADER(generated, ON, FN, BBFunctionOrderOP, bb_params)