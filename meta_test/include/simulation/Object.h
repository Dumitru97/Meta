#pragma once
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
