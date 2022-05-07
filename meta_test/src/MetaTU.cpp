#include "../include/MetaArguments1.h"
#include "../include/simulation/TestVariables.h"
#include "../include/simulation/DefaultCallOrder.h"

void CallOptimizedOrder() {
	META_CALL_FUNCTIONS_OPTIMIZED(ON, FN);
}

void InitArgumentVariables(int acts, int objs) {
	InitVariables(acts, objs);
}

void CallDefaultOrder1Wrapper() {
	CallDefaultOrder1();
}

void CallDefaultOrder2Wrapper() {
	CallDefaultOrder2();
}