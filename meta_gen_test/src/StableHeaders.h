#pragma once
#include <string>
#include <fstream>
#include "Shared.h"

namespace StableHeaders {

	void CreatePositionHeader() {
		std::ofstream file(OUTPUT_DIR "Position.h");
		file << 
R"(#pragma once
#include <math.h>
#include <Vc/Vc>
using Vc::float_v;

struct position {
	float_v x;
	float_v y;
};

//float distsq(const position& p1, const position& p2) {
//	return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
//}

//float dist(const position& p1, const position& p2) {
//	return sqrtf(distsq(p1, p2));
//}
)";
	}

	void CreateActorHeader() {
		std::ofstream file(OUTPUT_DIR "Actor.h");
		file <<
R"(#pragma once
#include <vector>
#include "Position.h"

struct Actor {
	position pos;
};

using Actors = std::vector<Actor>;
)";
	}
	void CreateObjectHeader() {
		std::ofstream file(OUTPUT_DIR "Object.h");
		file <<
R"(#pragma once
#include <vector>
#include "Position.h"
struct Object {
	position pos;
};

struct ObjectWrapper {
	Object obj;
	Object obj_next;
};

using Objects = std::vector<ObjectWrapper>;
)";
	}

	void CreateWorldHeader() {
		std::ofstream file(OUTPUT_DIR "World.h");
		file <<
R"(#pragma once

constexpr int world_width_bits = 5;
constexpr int world_height_bits = 5;

constexpr int world_width = 1 << world_width_bits;
constexpr int world_height = 1 << world_height_bits;

template<int bits>
inline int mod_int_pow_2(int val) {
    return val & ((1 << bits) - 1);
}

template<int bits>
inline float mod_float_pow_2(float val) {
    const float frac = val - (int)val;
    const int mod = mod_int_pow_2<bits>((int)val);

    const int neg = (1 << bits) * ((frac < -FLT_EPSILON * 100) && (mod == 0));
    return neg + mod + frac;
}

inline float mod_width(float val) {
	return mod_float_pow_2<world_width_bits>(val);
}

inline float mod_height(float val) {
	return mod_float_pow_2<world_height_bits>(val);
}

inline unsigned world_mat[world_height][world_width]{};
using World = unsigned(*)[world_width];
inline World world = world_mat;
)";
	}

	void CreateStableHeaders() {
		CreatePositionHeader();
		CreateActorHeader();
		CreateObjectHeader();
		CreateWorldHeader();
	}

}