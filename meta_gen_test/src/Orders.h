#pragma once
#include <string>
#include <fstream>
#include "Shared.h"

namespace Orders {
	std::string ObjectOrders(int obj_idx)
	{
		return std::format(
			"struct OObject{0}Moved {{}};" nl
			"struct OObject{0}Wrap : OObject{0}Moved {{}};" nl
			"struct OObject{0}Draw : OObject{0}Wrap {{}};" nl
			"struct OObject{0}UpdateCurrent : OObject{0}Wrap {{}};" nl2
			, obj_idx);
	}

	std::string ActorOrders(int act_idx) {
		return std::format(
			"struct OActor{0}Update {{}};" nl
			"struct OActor{0}UpdateDone : OActor{0}Update {{}};" nl2
			, act_idx
		);
	}

	void ConstructOrdersHeader() {
		std::ofstream file(OUTPUT_DIR "SharedOrders.h");
		
		file << "#pragma once" nl;
		file << OpenNamespace("ON");

		for (int k = 1; k <= obj_num; ++k)
			file << ObjectOrders(k);

		for (int k = 1; k <= actor_num; ++k)
			file << ActorOrders(k);
	
		file << CloseNamespace("ON");
		file.close();
	}
}