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
void CallOptimizedOrder();
void InitArgumentVariables(int acts, int objs);
void CallDefaultOrder1Wrapper();
void CallDefaultOrder2Wrapper();

// Prototypes
std::pair<float, float> TestWith(int acts, int objs, int reps);
void PrintResultTable(const std::vector<int>& acts_arr,
	                  const std::vector<int>& objs_arr,
	                  const std::vector<std::pair<float, float>>& result_mat);

int main() {
	std::vector<int> acts_arr{ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
	std::vector<int> objs_arr{ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
	std::vector<std::pair<float,float>> result_mat;
	const int work = 8;

	for (int i = 0; i < acts_arr.size(); ++i)
		for (int j = 0; j < objs_arr.size(); ++j) {
			const int acts = acts_arr[i];
			const int objs = objs_arr[j];
			const int reps = work * (acts_arr.back() / acts_arr[i]) * (objs_arr.back() / objs_arr[j]);

			auto results = TestWith(acts, objs, reps);
			result_mat.push_back(results);
		}

	PrintResultTable(acts_arr, objs_arr, result_mat);

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
	std::cout << "Result: " << res << '\t';
}

double BenchmarkSimulation(auto&& func, int reps) {
	auto t1 = high_resolution_clock::now();
	func(reps);
	auto t2 = high_resolution_clock::now();
	auto dur = duration_cast<nanoseconds>(t2 - t1);

	CheckSimulationResult();
	std::cout << "Time(ns)/rep: " << (double)dur.count() / reps << '\n';
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
			CallDefaultOrder1Wrapper();
	else if constexpr (callOrderType == DEFAULT2)
		for (int i = 0; i < reps; ++i)
			CallDefaultOrder2Wrapper();
	else if constexpr (callOrderType == OPTIMIZED)
		for (int i = 0; i < reps; ++i)
			CallOptimizedOrder();
}

std::pair<float, float> TestWith(int acts, int objs, int reps) {
	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "DefOrder1", reps);
	auto r1 = BenchmarkSimulation(RunSimulation<DEFAULT1>, reps);

	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "DefOrder2", reps);
	auto r2 = BenchmarkSimulation(RunSimulation<DEFAULT2>, reps);

	InitArgumentVariables(acts, objs);
	std::cout << std::format("Benchmarking with acts={0}, objs={1}, optimiz={2}, reps={3}\t", acts, objs, "SA       ", reps);
	auto r3 = BenchmarkSimulation(RunSimulation<OPTIMIZED>, reps);

	auto compare = [](const char* a_str, const char* b_str, auto a, auto b, float print_thresh_perc) {
		float thresh = print_thresh_perc;
		float value;
		if (a < b) {
			value = (b / a - 1) * 100;
			if(value > thresh)
				std::cout << std::format("{} FASTER than {} by {}%", a_str, b_str, value) << '\n';
		}
		else {
			value = (a / b - 1) * 100;
			if (value > thresh)
				std::cout << std::format("{} SLOWER than {} by {}%", a_str, b_str, value) << '\n';
			value *= -1;
		}
		return value;
	};

	auto cmp_do1 = compare("SA", "DefOrder1", r3, r1, 1);
	auto cmp_do2 = compare("SA", "DefOrder2", r3, r2, 1);

	std::cout << '\n';

	return { cmp_do1, cmp_do2 };
}

void PrintResultTable(const std::vector<int>& acts_arr,
	const std::vector<int>& objs_arr,
	const std::vector<std::pair<float, float>>& result_mat)
{
	auto to_str_n = [](auto value, auto n) {
		auto string = std::to_string(value);
		string.resize(std::min(n, (int)string.size()));
		return string;
	};

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
		static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

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
		r = g = b = 128;

		auto color_add = std::clamp(std::abs(value * color_per_unit), 0.0f, 127.0f);
		if (value > 0)
			g += color_add;
		else
			r += color_add;

		//Get prev
		DWORD dwMode = 0;
		GetConsoleMode(hConsole, &dwMode);

		// Set new
		SetConsoleMode(hConsole, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

		std::cout << std::format("\x1b[38;2;{0};{1};{2}m", r, g, b);
		cout_value();
		std::cout << std::format("\x1b[38;2;{0};{1};{2}m", 255, 255, 255);

		// Set prev
		SetConsoleMode(hConsole, dwMode);
#endif
	};

	auto calc_color_per_unit_pair = [](const auto& result_mat) {
		auto calc_color_per_unit = [](const auto& result_mat, auto result_getter) {
			auto [minv, maxv] = std::minmax_element(result_mat.begin(), result_mat.end(), [&](const auto& a, const auto& b) { return result_getter(a) < result_getter(b); });
			auto absmax = std::max(std::abs(result_getter(*minv)), std::abs(result_getter(*maxv)));
			float color_per_unit = 127.0f / absmax;

			return 2 * color_per_unit;
		};

		auto cpu1 = calc_color_per_unit(result_mat, [](auto pair) {return pair.first; });
		auto cpu2 = calc_color_per_unit(result_mat, [](auto pair) {return pair.second; });

		return std::pair{ cpu1, cpu2 };
	};

	auto print_table = [&](auto print_table_value, auto result_getter) {
		auto cout_flags = std::cout.flags();
		const auto cellsp = 7;

		std::cout << '|' << std::setw(cellsp + 1) << std::setfill(' ') << std::right << '|';
		for (auto acts : acts_arr)
			std::cout << std::left << std::setw(cellsp) << std::setfill(' ') << to_str_n(acts, 4) << '|';
		std::cout << " acts\n";

		auto row_char_num = (acts_arr.size() + 1) * (cellsp + 1) + 1;
		std::cout << std::setw(row_char_num) << std::setfill('_') << '_' << '\n';

		for (auto objsIdx = 0; objsIdx < objs_arr.size(); ++objsIdx)
		{
			std::cout << '|' << std::setw(cellsp) << std::setfill(' ') << to_str_n(objs_arr[objsIdx], 4) << '|';
			for (int actsIdx = 0; actsIdx < acts_arr.size(); ++actsIdx)
			{
				print_table_value(objsIdx, acts_arr.size(), actsIdx, cellsp, result_getter);
				std::cout << '|';
			}
			std::cout << '\n';
		}
		std::cout << " objs\n\n";

		std::cout.flags(cout_flags);
	};

	auto cpus = calc_color_per_unit_pair(result_mat);

	auto cout_result_table_value = [&](auto objsIdx, auto acts_arr_size, auto actsIdx, auto cellsp, auto result_getter) {
		const auto result = result_getter(result_mat[objsIdx * acts_arr.size() + actsIdx]);
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
	for (auto objsIdx = 0; objsIdx < objs_arr.size(); ++objsIdx)
		for (int actsIdx = 0; actsIdx < acts_arr.size(); ++actsIdx)
			obj_act_usage_mat[objsIdx * acts_arr.size() + actsIdx] = calc_usage(acts_arr[actsIdx], objs_arr[objsIdx]);

	auto cout_usage_table_value = [&](auto objsIdx, auto acts_arr_size, auto actsIdx, auto cellsp, auto result_getter) {
		const auto result = result_getter(obj_act_usage_mat[objsIdx * acts_arr.size() + actsIdx]);

		std::string      value_str = std::to_string(result);
		std::string_view value_sv = std::string_view(value_str);

		if (value_str.find('.') != std::string::npos)
			value_sv = sv_n(value_sv, 5);

		std::cout << std::left << std::setw(cellsp) << std::setfill(' ') << value_sv;
	};

	print_table(cout_usage_table_value, [](auto x) {return x; });
	print_table(cout_usage_table_value, [&](auto x) {return x / (float)(acts_x_objs_loop_funcs + acts_loop_funcs + objs_loop_funcs); });
}
