#pragma once

#include <random>
#include "TestVariables.h"

inline int var_init_seed = 777;
inline std::mt19937 gen(var_init_seed);
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

//inline int min_objs = 30000;
//inline int max_objs = 40000;
//
//inline int min_acts = 10;
//inline int max_acts = 100;
//inline int initMode;
//
//inline int gen_act_num() {
//	if (initMode != 0)
//		return rand_int(min_acts, max_acts);
//	else
//		return min_acts;
//}
//
//inline int gen_obj_num() {
//	if (initMode != 0)
//		return rand_int(min_objs, max_objs);
//	else
//		return min_objs;
//}


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

void InitVariables(int acts_, int objs_) {
	acts = acts_;
	objs = objs_;
	
	int total_actor_len = 0;
	int len = 0;
	len = gen_act_num();
	total_actor_len += len;
	InitActorVec(Actors1_gv, len);

	int total_obj_len = 0;

	len = gen_obj_num();
	total_obj_len += len;
	InitObjectVec(Objects1_gv, len);

	len = gen_obj_num();
	total_obj_len += len;
	InitObjectVec(Objects2_gv, len);

}
