#include <chrono>
#include <vector>
#include <iostream>
#include <format>
#include <sstream>
#include "include/simulation/World.h"
#include "include/simulation/DefaultCallOrderFuncNames.h"

#if defined(WIN32) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

using namespace std::chrono;

// Defined in MetaTU.cpp
void CallOptimizedOrderSA();
void CallOptimizedOrderBB();
void InitArgumentVariables(int acts, int objs);
void CallDefaultOrder1Wrapper();
void CallDefaultOrder2Wrapper();

// Local definitions
enum CallOrderType : int {
	DEFAULT1,
	DEFAULT2,
	SA,
	BB
};

// Prototypes
template<CallOrderType callOrderType>
std::pair<float, float> TestWith(int acts, int objs, size_t reps, const char* opti_name);

template<CallOrderType callOrderType>
void RunSimulation(size_t reps);

double BenchmarkSimulation(auto&& func, size_t reps, bool probe = false);

void PrintResultTable(const std::vector<int>& acts_arr,
	                  const std::vector<int>& objs_arr,
	                  const std::vector<std::pair<float, float>>& result_mat);

void sink(const auto& var) {
	volatile auto sinkhole = var;
}

double interp(double a, double b, double t, double p) {
	double nt = -1;
	while (p != 0) {
		nt *= (t - 1.0f);
		--p;
	}
	return a * (1.0f - nt) + b * nt - (a - b);
}

int main() {
#ifdef _WIN32
	system("color F0");
#endif

	auto bench = [] <CallOrderType callOrderType> (const char* opti_name) {
		std::vector<int> acts_arr{ 1, 2, 4, 8, 16, 32, 64, 128/*, 256, 512, 1024, 2048, 4096, 8192, 8192 * 2, 8192 * 4, 8192 * 8, 8192 * 16*/};
		std::vector<int> objs_arr{ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 8192 * 2, 8192 * 4, 8192 * 8, 8192 * 16 };
		std::vector<std::pair<float, float>> result_mat;

		constexpr double time_per_comb = 2e9;

		for (int i = 0; i < acts_arr.size(); ++i) {
			bool estimate_reps = true;
			for (int j = 0; j < objs_arr.size(); ++j) {
				const int acts = acts_arr[i];
				const int objs = objs_arr[j];

				size_t reps = 2ull;
				if (estimate_reps) {
					InitArgumentVariables(acts, objs);
					const float min = 1.5f;
					const float max = std::lerp(2500.0f, min, (float)(i + 1) / acts_arr.size());
					const size_t timing_reps = interp(max, min, (float)j / objs_arr.size(), 6);
					auto time_per_rep = BenchmarkSimulation(RunSimulation<DEFAULT1>, timing_reps, true);
					size_t est_reps = time_per_comb / time_per_rep;

					if (est_reps <= reps)
						estimate_reps = false;
					reps = std::max(reps, est_reps);
				}

				auto results = TestWith<callOrderType>(acts, objs, reps, opti_name);
				result_mat.push_back(results);
			}
		}

		PrintResultTable(acts_arr, objs_arr, result_mat);
	};

	bench.template operator()<CallOrderType::BB>("BB       ");
	bench.template operator()<CallOrderType::SA>("SA       ");

#ifdef _WIN32
	int wait;
	std::cin >> wait;
#endif
	return 0;
}

long long SimulationResult() {
	long long res = 0;
	for (int i = 0; i < (int)world_height; ++i)
		for (int j = 0; j < (int)world_width; ++j) {
			sink(world_mat[i][j]);
			res += world_mat[i][j];
		}
	return res;
}

double BenchmarkSimulation(auto&& func, size_t reps, bool probe) {
	auto t1 = high_resolution_clock::now();
	func(reps);
	auto t2 = high_resolution_clock::now();
	auto dur = duration_cast<nanoseconds>(t2 - t1);

	if (!probe) {
		auto res = SimulationResult();
		std::cout << "Result: " << res << '\t';
		std::cout << "Time(ns)/rep: " << (double)dur.count() / reps << '\n';
	}
	else {
		SimulationResult();
		std::cout << "Probing. reps=" << reps << '\t' << "Time(ns)/rep: " << (double)dur.count() / reps << '\n';
	}
	return (double)dur.count() / reps;
}

template<CallOrderType callOrderType>
void RunSimulation(size_t reps) {
	for (size_t i = 0; i < reps; ++i)
		if constexpr (callOrderType == DEFAULT1)
			CallDefaultOrder1Wrapper();
		else if constexpr (callOrderType == DEFAULT2)
			CallDefaultOrder2Wrapper();
		else if constexpr (callOrderType == SA)
			CallOptimizedOrderSA();
		else if constexpr (callOrderType == BB)
			CallOptimizedOrderBB();
}

template<CallOrderType callOrderType>
std::pair<float, float> TestWith(int acts, int objs, size_t reps, const char* opti_name) {
	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "DefOrder1", reps);
	auto r1 = BenchmarkSimulation(RunSimulation<DEFAULT1>, reps);

	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "DefOrder2", reps);
	auto r2 = BenchmarkSimulation(RunSimulation<DEFAULT2>, reps);

	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, opti_name, reps);
	auto r3 = BenchmarkSimulation(RunSimulation<callOrderType>, reps);

	auto compare = [](const char* a_str, const char* b_str, auto a, auto b, float print_thresh_perc) {
		float thresh = print_thresh_perc;
		float value = ((b - a) / a) * 100;

		if (std::abs(value) > std::abs(thresh)) {
			if (value < 0)
				std::cout << std::format("{} FASTER than {} by {}%", b_str, a_str, value) << '\n';
			else 
				std::cout << std::format("{} SLOWER than {} by {}%", b_str, a_str, value) << '\n';
		}

		value *= -1; // Positive values mean cost decrease 
		return value;
	};

	auto cmp_do1 = compare("DefOrder1", opti_name, r1, r3, 1);
	auto cmp_do2 = compare("DefOrder2", opti_name, r2, r3, 1);

	std::cout << '\n';

	return { cmp_do1, cmp_do2 };
}

void PrintResultTable(const std::vector<int>& acts_arr,
					  const std::vector<int>& objs_arr,
					  const std::vector<std::pair<float, float>>& result_mat)
{
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD dwMode = 0;
	GetConsoleMode(hConsole, &dwMode);
	SetConsoleMode(hConsole, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	constexpr unsigned char fg_val = 80;
	constexpr unsigned char bg_val = 255;

	std::cout << std::format("\x1b[48;2;{0};{1};{2}m", bg_val, bg_val, bg_val);
	std::cout << std::format("\x1b[38;2;{0};{1};{2}m", fg_val, fg_val, fg_val);

	auto sv_n = [](auto sv, size_t n) {
		return std::string_view{ sv.data(), std::min(n, sv.size()) };
	};

	auto get_whole_frac = [](const auto& value_str) {
		std::string_view sv(value_str);
		auto sep = sv.find('.');

		if (sep != sv.npos)
			return std::pair{ sv.substr(0, sep), sv.substr(sep + 1, sv.size()) };
		else
			return std::pair{ sv, std::string_view{} };
	};

	auto cout_gr_color = [&](auto value, int cellsp, auto color_per_unit) {
#if defined(WIN32) || defined(_WIN32)

		auto cout_value = [&]() {
			std::string value_str = std::to_string(value);
			auto [whole, frac] = get_whole_frac(value_str);

			int wholesp = std::ceil(cellsp / 2.0f);
			int fracsp = cellsp - wholesp - 1;

			whole = sv_n(whole, wholesp);
			frac = sv_n(frac, fracsp);

			std::cout << std::right << std::setw(wholesp) << std::setfill(' ') << whole;
			std::cout << '.';
			std::cout << std::left << std::setw(fracsp) << std::setfill(' ') << frac;
		};


		unsigned char r, g, b;
		r = g = b = fg_val;

		float th = 1.5f;
		auto color_add = std::clamp(std::abs((value - th) * color_per_unit), 0.0f, 255.0f - fg_val);
		auto color_sub = std::clamp(std::abs((value - th) * color_per_unit), 0.0f, (float)fg_val);
		
		if (value > th) {
			g += color_add;
			r -= color_sub;
			b -= color_sub;
		}
		else if (value < -th) {
			r += color_add;
			g -= color_sub;
			b -= color_sub;
		}

		std::cout << std::format("\x1b[38;2;{0};{1};{2}m", r, g, b);
		cout_value();
		std::cout << std::format("\x1b[38;2;{0};{1};{2}m", fg_val, fg_val, fg_val);

#endif
	};

	auto calc_color_per_unit_pair = [](const auto& result_mat) {
		auto calc_color_per_unit = [](const auto& result_mat, auto result_getter) {
			auto [minv, maxv] = std::minmax_element(result_mat.begin(), result_mat.end(), [&](const auto& a, const auto& b) { return result_getter(a) < result_getter(b); });
			auto absmax = std::max(std::abs(result_getter(*minv)), std::abs(result_getter(*maxv)));
			float color_per_unit = (255.0f - fg_val) / absmax;

			return 2 * color_per_unit;
		};

		auto cpu1 = calc_color_per_unit(result_mat, [](auto pair) {return pair.first; });
		auto cpu2 = calc_color_per_unit(result_mat, [](auto pair) {return pair.second; });

		return std::pair{ cpu1, cpu2 };
	};

	auto print_table = [&](auto print_table_value, auto result_getter) {
		auto cout_flags = std::cout.flags();
		const auto cellsp = 7;

		std::cout << '|' << std::setw(cellsp) << std::setfill(' ') << std::left << "act\\ob" << std::right << '|';
		for (auto objs : objs_arr)
			std::cout << std::left << std::setw(cellsp) << std::setfill(' ') << std::to_string(objs) << '|';
		//std::cout << " objs\n";
		std::cout << "\n";

		auto row_char_num = (objs_arr.size() + 1) * (cellsp + 1) + 1;
		std::cout << std::setw(row_char_num) << std::setfill('_') << '_' << '\n';
		
		for (int actsIdx = 0; actsIdx < acts_arr.size(); ++actsIdx)
		{
			std::cout << '|' << std::setw(cellsp) << std::setfill(' ') << std::to_string(acts_arr[actsIdx]) << '|';
			for (auto objsIdx = 0; objsIdx < objs_arr.size(); ++objsIdx)
			{
				print_table_value(actsIdx, objs_arr.size(), objsIdx, cellsp, result_getter);
				std::cout << '|';
			}
			std::cout << '\n';
		}
		//std::cout << " acts\n\n";
		std::cout << "\n\n";

		std::cout.flags(cout_flags);
	};

	auto cpus = calc_color_per_unit_pair(result_mat);

	auto cout_result_table_value = [&](auto actsIdx, auto objs_arr_size, auto objsIdx, auto cellsp, auto result_getter) {
		const auto result = result_getter(result_mat[actsIdx * objs_arr_size + objsIdx]);
		cout_gr_color(result, cellsp, result_getter(cpus));
	};

	print_table(cout_result_table_value, [](auto pair) {return pair.first; });
	print_table(cout_result_table_value, [](auto pair) {return pair.second; });

	auto calc_usage = [&](auto acts, auto objs) {
		int sum = 0;
		sum += acts * objs * acts_x_objs_loop_funcs;
		sum += acts * acts_loop_funcs;
		sum += objs * objs_loop_funcs;
		return sum;
	};

	std::vector<int> obj_act_usage_mat(result_mat.size());
	for (int actsIdx = 0; actsIdx < acts_arr.size(); ++actsIdx)
		for (auto objsIdx = 0; objsIdx < objs_arr.size(); ++objsIdx)
			obj_act_usage_mat[actsIdx * objs_arr.size() + objsIdx] = calc_usage(acts_arr[actsIdx], objs_arr[objsIdx]);

	auto cout_usage_table_value = [&](auto actsIdx, auto objs_arr_size, auto objsIdx, auto cellsp, auto result_getter) {
		const auto result = result_getter(obj_act_usage_mat[actsIdx * objs_arr.size() + objsIdx]);

		std::string      value_str = std::to_string(result);
		std::string_view value_sv = std::string_view(value_str);

		if (value_str.find('.') != std::string::npos)
			value_sv = sv_n(value_sv, 5);

		std::cout << std::left << std::setw(cellsp) << std::setfill(' ') << value_sv;
	};

	print_table(cout_usage_table_value, [](auto x) { return x; });
	print_table(cout_usage_table_value, [&](auto x) { return x / (float)(acts_x_objs_loop_funcs + acts_loop_funcs + objs_loop_funcs); });

	std::cout << std::format("\x1b[48;2;{0};{1};{2}m", 12, 12, 12);
	std::cout << std::format("\x1b[38;2;{0};{1};{2}m", 204, 204, 204);
	SetConsoleMode(hConsole, dwMode);
}
