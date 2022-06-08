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

auto distsq(const position& p1, const position& p2) {
	const auto xd = (p1.x - p2.x);
	const auto yd = (p1.y - p2.y);
	return xd * xd + yd * yd;
}

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



//https://en.wikipedia.org/wiki/Permuted_congruential_generator
inline uint64_t mcg_state = 0xcafef00dd15ea5e5u;	// Must be odd

inline uint32_t pcg32_fast(void)
{
	uint64_t x = mcg_state;
	unsigned count = (unsigned)(x >> 61);	// 61 = 64 - 3

	mcg_state = x * 0xf13283ad;
	x ^= x >> 22;
	return (uint32_t)(x >> (22 + count));	// 22 = 32 - 3 - 7
}

//https://www.pcg-random.org/posts/bounded-rands.html
inline uint32_t bounded_rand(uint32_t range) {
	uint32_t x = pcg32_fast();
	uint64_t m = uint64_t(x) * uint64_t(range);
	return m >> 32;
}

)";
	}

	void CreateStableHeaders() {
		CreatePositionHeader();
		CreateActorHeader();
		CreateObjectHeader();
		CreateWorldHeader();
	}

}