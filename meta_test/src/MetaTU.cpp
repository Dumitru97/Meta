#include "../include/MetaArguments1.h"
#include "../include/MetaArguments2.h"
#include "../include/simulation/TestVariables.h"
#include "../include/simulation/DefaultCallOrder.h"

void CallOptimizedOrderSA() {
	META_CALL_FUNCTIONS_OPTIMIZED(ON, FN, SAFunctionOrderOP, sa_params);
}

void CallOptimizedOrderBB() {
	META_CALL_FUNCTIONS_OPTIMIZED(ON, FN, BBFunctionOrderOP, bb_params);
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