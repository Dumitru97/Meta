#include <filesystem>
#include <iostream>
#include <string>

#include "Actors.h"
#include "Objects.h"
#include "Orders.h"
#include "Includes.h"
#include "StableHeaders.h"
#include "Variables.h"
#include "DefaultCallOrder.h"

#define read_var(X)						\
std::cout << "Input value for "#X ": "; \
std::cin >> X;							\
std::cout << nl;


int main() {
	if constexpr (console_input) {
		read_var(actor_num);
		read_var(obj_num);
		read_var(seed);
	}

	bool exists = std::filesystem::exists(OUTPUT_DIR);
	if (exists) {
		std::filesystem::remove_all(OUTPUT_DIR);
		std::filesystem::create_directory(OUTPUT_DIR);
	}
	else
		std::filesystem::create_directory(OUTPUT_DIR);

	INIT_OBJ_ACT_ASOC_MAT();

	for (int i = 1; i <= obj_num; ++i)
		Objects::ConstructObjHeader(i);

	for (int i = 1; i <= actor_num; ++i)
		Actors::ConstructActHeader(i);

	Orders::ConstructOrdersHeader();
	Includes::ConstructIncludesHeader();
	StableHeaders::CreateStableHeaders();
	Variables::CreateVariablesHeader();
	DefaultCallOrder::CreateDefaultCallHeader();
}