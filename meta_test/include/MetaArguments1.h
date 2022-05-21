#pragma once
#include "meta.h"
#include "simulation/TestIncludes.h"

extern Meta::SAFunctionOrder::SAParams sa_params;
META_DEFINITIONS_ON_FN(ON, FN)
META_DEFINITIONS_ON_FN_OP(ON, FN, SAFunctionOrderOP)
META_PRECOMPUTE(ON, FN, SAFunctionOrderOP, sa_params)
META_CREATE_ARGUMENTS(ON, FN)
#include META_INCLUDE_IDXS_HEADER(generated, ON, FN, SAFunctionOrderOP, sa_params)