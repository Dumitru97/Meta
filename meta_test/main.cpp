#include <chrono>
#include <vector>
#include <iostream>
#include <format>
#include "include/simulation/World.h"
using namespace std::chrono;

// Defined in MetaTU.cpp
void CallOptimizedOrder();
void InitArgumentVariables(int acts, int objs);
void CallDefaultOrder1();
void CallDefaultOrder2();

// Prototypes
void TestWith(int acts, int objs, int reps);

int main() {

	std::vector<int> acts_arr{ 2, 4, 8, 16 };
	std::vector<int> objs_arr{ 2, 4, 8, 16 };
	std::vector<int> reps_arr{ 100'000 };

	for (int i = 0; i < acts_arr.size(); ++i)
		for (int j = 0; j < objs_arr.size(); ++j)
			for (int k = 0; k < reps_arr.size(); ++k) {
				const int acts = acts_arr[i];
				const int objs = objs_arr[j];
				const int reps = reps_arr[k];
				TestWith(acts, objs, reps);

				//TODO better separation of stable functions, have just the meta stuff in one file, and the other(less stable) in another
				/*OOPC_Init(actor_num, acts, obj_num, objs);
				std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "OOP", reps);
				Benchmark(CallTestOOP, false, reps);
				std::cout << "\n";*/
			}

#ifdef _WIN32
	system("pause");
#endif
	return 0;
}

void CheckSimulationResult() {
	long long res = 0;
	for (int i = 0; i < (int)world_height; ++i)
		for (int j = 0; j < (int)world_width; ++j)
			res += world_mat[i][j];
	std::cout << "Result: " << res << "\t";
}

double BenchmarkSimulation(auto&& func, int reps) {
	auto t1 = high_resolution_clock::now();
	func(reps);
	auto t2 = high_resolution_clock::now();
	auto dur = duration_cast<nanoseconds>(t2 - t1);

	CheckSimulationResult();
	std::cout << "Time(ns)/rep: " << (double)dur.count() / reps << "\n";
	return (double)dur.count() / reps;
}

enum CallOrderType {
	DEFAULT1,
	DEFAULT2,
	OPTIMIZED
};

template<CallOrderType callOrderType>
void RunSimulation(int reps) {
	if constexpr (callOrderType == DEFAULT1)
		for (int i = 0; i < reps; ++i)
			CallDefaultOrder1();
	else if constexpr (callOrderType == DEFAULT2)
		for (int i = 0; i < reps; ++i)
			CallDefaultOrder2();
	else if constexpr (callOrderType == OPTIMIZED)
		for (int i = 0; i < reps; ++i)
			CallOptimizedOrder();
}

void TestWith(int acts, int objs, int reps) {
	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "DefOrder1", reps);
	auto r1 = BenchmarkSimulation(RunSimulation<DEFAULT1>, reps);

	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "DefOrder2", reps);
	auto r2 = BenchmarkSimulation(RunSimulation<DEFAULT2>, reps);

	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "SA       ", reps);
	auto r3 = BenchmarkSimulation(RunSimulation<OPTIMIZED>, reps);

	if (r3 < r2 && r3 < r1) {
		std::cout << "FASTER" "\n";
		//diff
	}
	std::cout << "\n";
}

