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
#include <math.h>

constexpr int world_width_int = 20;
constexpr int world_height_int = 20;

constexpr float world_width = world_width_int - FLT_EPSILON * 10;
constexpr float world_height = world_height_int - FLT_EPSILON * 10;

inline float fmodf_width(float val) {
	return fmodf(val, world_width);
}

inline float fmodf_height(float val) {
	return fmodf(val, world_height);
}

inline unsigned world_mat[world_height_int][world_width_int]{};
using World = unsigned(*)[world_width_int];
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