#pragma once
#include <string>
#include <fstream>
#include "Shared.h"

namespace Variables {
	auto InitFuncHeader() {
		std::string str = 
R"(#include <random>
#include "TestVariables.h"
)";
		str += std::format("inline int var_init_seed = {};" nl, TEST_VAR_SEED);
		str +=
R"(inline std::mt19937 gen(var_init_seed);
inline std::uniform_int_distribution<int> uid;
inline int rand_int(int min, int max) {
	using param_type = decltype(uid)::param_type;
	uid.param(param_type{ min, max });
	return uid(gen);
}

inline std::uniform_real_distribution<float> urd;
inline float rand_float(float min, float max) {
	using param_type = decltype(urd)::param_type;
	urd.param(param_type{ min, max });
	return urd(gen);
}

inline int objs;
inline int acts;
inline int gen_act_num() {
	return acts;
}

inline int gen_obj_num() {
	return objs;
}

inline float gen_x_pos() {
	return rand_float(0, world_width);
}

inline float gen_y_pos() {
	return rand_float(0, world_height);
}

using Vc::float_v;
void InitActorVec(Actors& vec, int len) {
	vec.resize(len);
	for (int i = 0; i < len; ++i) {
		std::array<float, float_v::Size> vals;
		for (int j = 0; j < float_v::Size; ++j)
			vals[j] = gen_x_pos();

		vec[i].pos.x.load(vals.data());

		for (int j = 0; j < float_v::Size; ++j)
			vals[j] = gen_y_pos();

		vec[i].pos.y.load(vals.data());
	}
}

void InitObjectVec(Objects& vec, int len) {
	vec.resize(len);
	for (int i = 0; i < len; ++i)  {
		std::array<float, float_v::Size> vals;
		for (int j = 0; j < float_v::Size; ++j)
			vals[j] = gen_x_pos();

		vec[i].obj.pos.x.load(vals.data());

		for (int j = 0; j < float_v::Size; ++j)
			vals[j] = gen_y_pos();

		vec[i].obj.pos.y.load(vals.data());
		vec[i].obj_next = vec[i].obj;
	}
}

)";


		str += R"(void InitVariables(int acts_, int objs_) {
	for (int i = 0; i < (int)world_height; ++i)
		for (int j = 0; j < (int)world_width; ++j)
			world_mat[i][j] = {};

	gen.seed(var_init_seed);
	acts = acts_;
	objs = objs_;
	int len = 0;

	// Init actors
	int total_actor_len = 0;

)";

		bool print = print_acts_objs_len;
		for (int i = 1; i <= actor_num; ++i) {
			str += tab "len = gen_act_num();" nl
				   tab "total_actor_len += len;" nl;
			if (print)
			str += std::format(tab R"(std::cout << "Actors{} len = " << len << "\n";)" nl, i);
			str += std::format(tab R"(InitActorVec(Actors{}_gv, len);)" nl2, i);
		}

		if (print)
			str += tab R"(std::cout << "\n" << "Total Actors len = " << total_actor_len << "\n\n";)" nl;

		str += 
R"(
	// Init objects
	int total_obj_len = 0;
)" nl;
		for (int i = 1; i <= obj_num; ++i) {
			str += tab "len = gen_obj_num();" nl
				   tab "total_obj_len += len;" nl;
			if (print)
			str += std::format(tab R"(std::cout << "Objects{} len = " << len << "\n";)" nl, i);
			str += std::format(tab R"(InitObjectVec(Objects{}_gv, len);)" nl2, i);
		}

		if (print)
			str += tab R"(std::cout << "\n" << "Total Objects len = " << total_obj_len << "\n\n";)" nl;

		str += "}\n";
		return str;
	}

	auto CreateConstantsHeader() {
		std::string str;

#if defined(FORCE_INLINE)
		str += "#define TEST_INLINE __attribute__((__always_inline__))" nl2;
#elif defined(FORCE_NOINLINE)
		str += "#define TEST_INLINE __attribute__((noinline))" nl2;
#else
		str += "#define TEST_INLINE " nl2;
#endif

		str += std::format(
			"constexpr int actor_num = {0};" nl
			"constexpr int obj_num = {1};" nl,
			actor_num, obj_num
		);

		return str;
	}

	void CreateVariablesHeader() {
		std::ofstream file1(OUTPUT_DIR "TestVariables.h");
		file1 << "#pragma once" nl2;
		file1 << InitFuncHeader();


		std::ofstream file2(OUTPUT_DIR "Constants.h");
		file2 << "#pragma once" nl2;
		file2 << CreateConstantsHeader();
	}
}