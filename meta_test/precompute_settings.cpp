#include "optimizer/SimulatedAnnealing_Implementation1.h"
#include "optimizer/BranchAndBound1.h"

Meta::SAFunctionOrder::SAParams sa_params = { .reps = 200, .temp = 25, .reps_increment = 20, .temp_decrement = 0.1f, .pow_mult = 20 };
Meta::BBFunctionOrder::BBParams bb_params = { .pqueueThresh = 5000ull, .cullRatio = 0.55f };
