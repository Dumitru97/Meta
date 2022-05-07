#include "optimizer/SimulatedAnnealing_Implementation1.h"

namespace Meta {
	namespace SAFunctionOrder {
		SAParams sa_params = { .reps = 500, .temp = 25, .reps_increment = 20, .temp_decrement = 0.15f, .pow_mult = 20 };
	}
}