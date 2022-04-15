#pragma once
#include <math.h>

constexpr float world_width = 20.0f - FLT_EPSILON * 10;
constexpr float world_height = 20.0f - FLT_EPSILON * 10;

inline float fmodf_width(float val) {
	return fmodf(val, world_width);
}

inline float fmodf_height(float val) {
	return fmodf(val, world_height);
}

constexpr int world_width_int = 20;
constexpr int world_height_int = 20;

inline unsigned world_mat[world_height_int][world_width_int]{};
using World = unsigned(*)[world_width_int];
inline World world = world_mat;
