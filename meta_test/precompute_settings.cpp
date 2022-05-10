#include "optimizer/SimulatedAnnealing_Implementation1.h"

namespace Meta {
	namespace SAFunctionOrder {
		SAParams sa_params = { .reps = 100, .temp = 25, .reps_increment = 250, .temp_decrement = 0.08f, .pow_mult = 20 };
	}
}