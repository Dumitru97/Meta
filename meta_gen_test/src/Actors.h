#pragma once
#include <fstream>
#include <string>
#include <format>
#include "Shared.h"

namespace Actors {
	std::string ActorUpdateFunc(int act_idx) {
		const float x_adv = gen_act_x_adv();
		const float y_adv = gen_act_y_adv();
		const Axis axis = gen_axis();

		// Constructing format
		std::string fmt =
"INLINE void Actors{0}Update(Actors{0}&acts{0}, OActor{0}Update) {{" nl;
		if(print_function_name)
			fmt += tab R"(std::cout << __PRETTY_FUNCTION__ << "\n";)" nl2;

		if (axis == Axis::X)
			fmt += tab "constexpr float x_adv = {1}f;" nl;
		else if (axis == Axis::Y)
			fmt += tab "constexpr float y_adv = {1}f;" nl;
		else //if (axis == Axis::XY)
			fmt += tab "constexpr float x_adv = {1}f;" nl
				   tab "constexpr float y_adv = {2}f;" nl;

		fmt += tab "for (auto& act : acts{0}) {{" nl;
		if (axis & Axis::X)
			fmt += tab2 "act.pos.x += x_adv;" nl;
		if (axis & Axis::Y)
			fmt += tab2 "act.pos.y += y_adv;" nl;

		fmt += tab "}}" nl
			"}}" nl;

		// Getting formatted string
		if (axis == Axis::X)
			return std::vformat(fmt, std::make_format_args(act_idx, x_adv));
		if (axis == Axis::Y)
			return std::vformat(fmt, std::make_format_args(act_idx, x_adv));
		else //if (axis == Axis::XY)
			return std::vformat(fmt, std::make_format_args(act_idx, x_adv, y_adv));
	}

	void ConstructActHeader(const int idx) {
		const std::string act_str("Actor" + std::to_string(idx));
		std::ofstream file(OUTPUT_DIR + act_str + ".h");

		file << "#pragma once" nl;
		file << UsingAlias("Actors" + std::to_string(idx), "Actors");

		file << UsingNamespace("ON");
		file << ActorUpdateFunc(idx);

		file.close();
	}
}