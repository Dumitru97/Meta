#include "optimizer/SimulatedAnnealing_Implementation1.h"
#include "optimizer/BranchAndBound1.h"

Meta::SAFunctionOrder::SAParams sa_params = { .reps = 300, .temp = 25, .reps_increment = 250, .temp_decrement = 0.1f, .pow_mult = 20 };
//Meta::BBFunctionOrder::BBParams bb_params = { .pqueueThresh = 1000, .cullRatio = 0.3f };
Meta::BBFunctionOrder::BBParams bb_params = { .pqueueThresh = 500, .cullRatio = 0.7f };
