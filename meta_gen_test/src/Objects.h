#pragma once
#include <fstream>
#include <string>
#include <format>
#include "Shared.h"

namespace Objects {
	std::string ObjMovedByActFuncs(int obj_idx, int act_idx) {
		const float dist_tresh = gen_dist_thresh();
		const float x_adv = gen_obj_x_adv();
		const float y_adv = gen_obj_y_adv();
		const Axis axis = gen_axis();
		
		// Constructing format
		std::string fmt =
		"TEST_INLINE void Objs{0}MovedByActs{1}(Objects{0}& objs{0}, const Actors{1}& acts{1}, OObject{0}MovedBy, OActor{1}UpdateDone) {{" nl;
		if(print_function_name)
			fmt += tab R"(std::cout << __PRETTY_FUNCTION__ << "\n";)" nl2;

		fmt += tab "constexpr float dist_tresh = {2}f;";
		fmt +=
R"(
	for (const auto& act : acts{1})
		for (int i = 0; i < objs{0}.size(); ++i) {{
			const auto& obj = objs{0}[i].obj;
			auto& obj_next = objs{0}[i].obj_next;

			const auto mask = distsq(act.pos, obj.pos) > dist_tresh * dist_tresh;
)";
		if (axis == Axis::X)
			fmt += tab3 "obj_next.pos.x(mask) += {3}f;" nl;
		else if (axis == Axis::Y)
			fmt += tab3 "obj_next.pos.y(mask) += {3}f;" nl;
		else //if (axis == Axis::XY)
			fmt += tab3 "obj_next.pos.x(mask) += {3}f;" nl
				   tab3 "obj_next.pos.y(mask) += {4}f;" nl;

		fmt += tab2 "}}" nl
			   tab "}}" nl2;

		// Getting formatted string
		if (axis == Axis::X)
			return std::vformat(fmt, std::make_format_args(obj_idx, act_idx, dist_tresh, x_adv));
		if (axis == Axis::Y)
			return std::vformat(fmt, std::make_format_args(obj_idx, act_idx, dist_tresh, y_adv));
		else //if (axis == Axis::XY)
			return std::vformat(fmt, std::make_format_args(obj_idx, act_idx, dist_tresh, x_adv, y_adv));
	}

	std::string ObjsWorldWrapFunc(int obj_idx) {
		std::string fmt =
		"TEST_INLINE void Objs{0}WorldWrap(Objects{0}& objs{0}, OObject{0}Wrap) {{" nl;
		if (print_function_name)
			fmt += tab R"(std::cout << __PRETTY_FUNCTION__ << "\n";)" nl;

		fmt +=
R"(
	for (auto& obj_wrapper : objs{0}) {{
		auto& objn = obj_wrapper.obj_next;

		objn.pos.x = objn.pos.x.apply(mod_width);
		objn.pos.y = objn.pos.y.apply(mod_height);
	}}
}})" nl2;

		return std::vformat(fmt, std::make_format_args(obj_idx));
	}

	std::string ObjsDrawFunc(int obj_idx)
	{
		const int paint = gen_paint();
		std::string fmt =
		"TEST_INLINE void Objs{0}Draw(const Objects{0}& objs{0}, OObject{0}Draw) {{" nl;
		if (print_function_name)
			fmt += tab R"(std::cout << __PRETTY_FUNCTION__ << "\n";)" nl;

		fmt +=
R"(
	constexpr int paint_val = {1};
	for (const auto& obj_wrapper : objs{0}) {{
		const auto& objn = obj_wrapper.obj_next;

		for(int i = 0; i < float_v::Size; ++i)
			world[(int)objn.pos.x[i]][(int)objn.pos.y[i]] += paint_val;
	}}
}})" nl2;

		return std::vformat(fmt, std::make_format_args(obj_idx, paint));
	}

	std::string ObjsUpdateFunc(int obj_idx)
	{
		std::string fmt =
			"TEST_INLINE void Objs{0}UpdateCurrent(Objects{0}& objs{0}, OObject{0}UpdateCurrent) {{" nl;
		if (print_function_name)
			fmt += tab R"(std::cout << __PRETTY_FUNCTION__ << "\n";)" nl;

		fmt +=
R"(
	for (auto& obj_wrapper : objs{0})
		obj_wrapper.obj = obj_wrapper.obj_next;
}})" nl2;

		return std::vformat(fmt, std::make_format_args(obj_idx));
	}

	void ConstructObjHeader(const int idx) {
		const std::string obj_str("Object" + std::to_string(idx));
		std::ofstream file(OUTPUT_DIR + obj_str + ".h");

		file << "#pragma once\n";
		file << UsingAlias("Objects" + std::to_string(idx), "Objects");
		file << UsingNamespace("ON");

		for (int i = 0; i < obj_act_asoc_mat[idx-1].size(); i++) {
			const int act_idx = obj_act_asoc_mat[idx-1][i];
			file << ObjMovedByActFuncs(idx, act_idx);
		}
		
		file << ObjsWorldWrapFunc(idx);
		file << ObjsDrawFunc(idx);
		file << ObjsUpdateFunc(idx);

		file.close();
	}
}