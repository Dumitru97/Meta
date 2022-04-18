#pragma once
#pragma once
#include <string>
#include <fstream>
#include "Shared.h"

namespace Includes {
	std::string Includes() {
		std::string includes =
R"(#ifndef NDEBUG
	#include <iostream>
#endif
#include "Constants.h"
#include "SharedOrders.h"
#include "Object.h"
#include "Actor.h"
#include "World.h"
namespace FN {)" nl;

		for (int i = 1; i <= actor_num; ++i)
			includes.append("#include \"Actor" + std::to_string(i) + ".h\"" nl);

		for (int i = 1; i <= obj_num; ++i)
			includes.append("#include \"Object" + std::to_string(i) + ".h\"" nl);

		includes.append("}" nl);
		return includes;
	}

	void ConstructIncludesHeader() {
		std::ofstream file(OUTPUT_DIR "TestIncludes.h");

		file << "#pragma once" nl;
		file << Includes();

		file.close();
	}
}