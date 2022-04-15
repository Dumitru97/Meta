#pragma once
#include <math.h>
#define INLINE /*__declspec(noinline)*/
//#define PRINT_FUNC
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
