#include "../include/MetaArguments1.h"
#include "../include/simulation/TestVariables.h"
#include "../include/simulation/DefaultCallOrder.h"

void CallOptimizedOrder() {
	META_CALL_FUNCTIONS_OPTIMIZED(ON, FN);
}

void InitArgumentVariables(int acts, int objs) {
	InitVariables(acts, objs);
}

void CallDefaultOrder1() {
	DefaultCallOrder1();
}

void CallDefaultOrder2() {
	DefaultCallOrder2();
}