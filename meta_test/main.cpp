#include "include/MetaArguments1.h"
#include <chrono>

void funcs() {
	META_CALL_FUNCTIONS_OPTIMIZED(ON, FN);
}

int main() {
	//InitVariables();

	//using std::chrono::high_resolution_clock;
	//using std::chrono::duration_cast;
	//using std::chrono::microseconds;
	//using std::chrono::milliseconds;

	//int reps, optimiz;
	//std::cout << "Set loop reps(int), apply optimization(0/1):\n";
	//std::cin >> reps >> optimiz;
	//auto t1 = high_resolution_clock::now();
	//int i = 0;

	//if (!optimiz)
	//	while (i++ < reps)
	//		InitMetaOrig();
	//else
	//	while (i++ < reps)
	//		InitMetaOptimFullCT();

	//auto t2 = high_resolution_clock::now();
	//auto dur = duration_cast<milliseconds>(t2 - t1);
	//std::cout << dur << "\n";

	//long long res = 0;
	//for (int i = 0; i < (int)world_height; ++i)
	//	for (int j = 0; j < (int)world_width; ++j)
	//		res += world_mat[i][j];
	//std::cout << "Result: " << res << "\n";
}