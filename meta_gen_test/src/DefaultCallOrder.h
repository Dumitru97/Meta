#pragma once
#include <string>
#include <fstream>
#include <format>
#include "Shared.h"

namespace DefaultCallOrder {

	void forLoop(std::ofstream& file, const char* fmt, int num) {
		for (int i = 1; i <= num; ++i)
			file << std::vformat(fmt, std::make_format_args(i));
		file << nl;
	}

	void forActors(std::ofstream& file, const char* fmt) {
		forLoop(file, fmt, actor_num);
	}

	void forObjects(std::ofstream& file, const char* fmt) {
		forLoop(file, fmt, obj_num);
	}

	void DefaultCallOrder1(std::ofstream& file) {
		file << "void DefaultCallOrder1() {" nl;

		forActors(file, tab "Actors{0}Update(Actors{0}_gv, {{}});" nl);

		for (int obj_idx = 1; obj_idx <= obj_num; ++obj_idx) {
			for (int i = 0; i < obj_act_asoc_mat[obj_idx - 1].size(); i++) {
				const int act_idx = obj_act_asoc_mat[obj_idx - 1][i];
				file << std::format(tab "Objs{0}MovedByActs{1}(Objects{0}_gv, Actors{1}_gv, {{}}, {{}});" nl, obj_idx, act_idx);
			}
			file << std::format(tab "Objs{0}WorldWrap(Objects{0}_gv, {{}}, {{}});"	nl, obj_idx);
			file << std::format(tab "Objs{0}Draw(Objects{0}_gv, {{}});"				nl, obj_idx);
			file << std::format(tab "Objs{0}UpdateCurrent(Objects{0}_gv, {{}});"	nl, obj_idx);
			file << nl;
		}

		file << "}" nl2;
	}

	void DefaultCallOrder2(std::ofstream& file) {
		file << "void DefaultCallOrder2() {" nl;

		forActors(file, tab "Actors{0}Update(Actors{0}_gv, {{}});" nl);

		for (int obj_idx = 1; obj_idx <= obj_num; ++obj_idx) {
			for (int i = 0; i < obj_act_asoc_mat[obj_idx - 1].size(); i++) {
				const int act_idx = obj_act_asoc_mat[obj_idx - 1][i];
				file << std::format(tab "Objs{0}MovedByActs{1}(Objects{0}_gv, Actors{1}_gv, {{}}, {{}});" nl, obj_idx, act_idx);
			}
			file << nl;
		}
		file << nl;

		forObjects(file, tab "Objs{0}WorldWrap(Objects{0}_gv, {{}}, {{}});" nl);
		forObjects(file, tab "Objs{0}Draw(Objects{0}_gv, {{}});"			nl);
		forObjects(file, tab "Objs{0}UpdateCurrent(Objects{0}_gv, {{}});"	nl);

		file << "}" nl2;
	}

	void CreateDefaultCallHeader() {
		std::ofstream file(OUTPUT_DIR "DefaultCallOrder.h");
		file << "#pragma once" nl;
		file << "using namespace FN;" nl2;
		DefaultCallOrder1(file);
		DefaultCallOrder2(file);
	}
}