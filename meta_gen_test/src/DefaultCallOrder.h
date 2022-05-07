#pragma once
#include <string>
#include <fstream>
#include <format>
#include "Shared.h"

namespace DefaultCallOrder {
	using FuncName = std::string;
	std::vector<FuncName> defaultOrder1;
	std::vector<FuncName> defaultOrder2;

	void addFuncName(std::ofstream& file, std::vector<FuncName>& defaultOrder, const char* prefix, const char* nameFmt, auto&&... fmtArgs) {
		auto funcName = std::vformat(nameFmt, std::make_format_args(std::forward<decltype(fmtArgs)>(fmtArgs)...));

		file << prefix << funcName;
		defaultOrder.push_back(std::move(funcName));
	}

	void addFuncParams(std::ofstream& file, const char* paramFmt, auto&&... fmtArgs) {
		auto funcParams = std::vformat(paramFmt, std::make_format_args(std::forward<decltype(fmtArgs)>(fmtArgs)...));
		file << funcParams;
	}


	void forLoop(std::ofstream& file, std::vector<FuncName>& defaultOrder, const char* prefix, const char* nameFmt, const char* paramFmt, int num) {
		for (int i = 1; i <= num; ++i) {
			auto funcName = std::vformat(nameFmt, std::make_format_args(i));
			auto funcParams = std::vformat(paramFmt, std::make_format_args(i));

			file << prefix << funcName << funcParams;
			defaultOrder.push_back(std::move(funcName));
		}
		file << nl;
	}

	void forActors(std::ofstream& file, std::vector<FuncName>& defaultOrder, const char* prefix, const char* nameFmt, const char* paramFmt) {
		forLoop(file, defaultOrder, prefix, nameFmt, paramFmt, actor_num);
	}

	void forObjects(std::ofstream& file, std::vector<FuncName>& defaultOrder, const char* prefix, const char* nameFmt, const char* paramFmt) {
		forLoop(file, defaultOrder, prefix, nameFmt, paramFmt, obj_num);
	}

	void DefaultCallOrder1(std::ofstream& file) {
		file << "void CallDefaultOrder1() {" nl;

		forActors(file, defaultOrder1, tab, "Actors{0}Update", "(Actors{0}_gv, {{}});" nl);

		for (int obj_idx = 1; obj_idx <= obj_num; ++obj_idx) {
			for (int i = 0; i < obj_act_asoc_mat[obj_idx - 1].size(); i++) {
				const int act_idx = obj_act_asoc_mat[obj_idx - 1][i];
				addFuncName  (file, defaultOrder1, tab, "Objs{0}MovedByActs{1}", obj_idx, act_idx);
				addFuncParams(file, "(Objects{0}_gv, Actors{1}_gv, {{}}, {{}});" nl, obj_idx, act_idx);
			}
			addFuncName  (file, defaultOrder1, tab, "Objs{0}WorldWrap", obj_idx);
			addFuncParams(file, "(Objects{0}_gv, {{}}, {{}});" nl, obj_idx);

			addFuncName  (file, defaultOrder1, tab, "Objs{0}Draw", obj_idx);
			addFuncParams(file, "(Objects{0}_gv, {{}});" nl, obj_idx);

			addFuncName  (file, defaultOrder1, tab, "Objs{0}UpdateCurrent", obj_idx);
			addFuncParams(file, "(Objects{0}_gv, {{}});" nl, obj_idx);

			file << nl;
		}

		file << "}" nl2;
	}

	void DefaultCallOrder2(std::ofstream& file) {
		file << "void CallDefaultOrder2() {" nl;

		forActors(file, defaultOrder2, tab, "Actors{0}Update", "(Actors{0}_gv, {{}});" nl);

		for (int obj_idx = 1; obj_idx <= obj_num; ++obj_idx) {
			for (int i = 0; i < obj_act_asoc_mat[obj_idx - 1].size(); i++) {
				const int act_idx = obj_act_asoc_mat[obj_idx - 1][i];
				addFuncName  (file, defaultOrder2, tab, "Objs{0}MovedByActs{1}", obj_idx, act_idx);
				addFuncParams(file, "(Objects{0}_gv, Actors{1}_gv, {{}}, {{}});" nl, obj_idx, act_idx);
			}
			file << nl;
		}
		file << nl;

		forObjects(file, defaultOrder2, tab, "Objs{0}WorldWrap"    , "(Objects{0}_gv, {{}}, {{}});" nl);
		forObjects(file, defaultOrder2, tab, "Objs{0}Draw"         , "(Objects{0}_gv, {{}});"       nl);
		forObjects(file, defaultOrder2, tab, "Objs{0}UpdateCurrent", "(Objects{0}_gv, {{}});"       nl);

		file << "}" nl2;
	}

	void CreateDefaultCallHeader() {
		std::ofstream file(OUTPUT_DIR "DefaultCallOrder.h");
		file << "#pragma once" nl;
		file << "using namespace FN;" nl2;
		DefaultCallOrder1(file);
		DefaultCallOrder2(file);

		std::ofstream file2(OUTPUT_DIR "DefaultCallOrderFuncNames.h");
		file2 << std::format("inline std::array<const char*, {0}> defaultOrder1{{ {1} }};" nl2, defaultOrder1.size(), QuoteContainer{ defaultOrder1 });
		file2 << std::format("inline std::array<const char*, {0}> defaultOrder2{{ {1} }};" nl2, defaultOrder2.size(), QuoteContainer{ defaultOrder2 });
	}
}