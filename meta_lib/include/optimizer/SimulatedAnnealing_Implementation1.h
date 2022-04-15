#pragma once
#include "../1_Orders.h"
#include "../3_Functions.h"
#include "SimulatedAnnealing.h"
#include "CostFunctions.h"
#include <limits>
#include <vector>
#include <algorithm>

namespace Meta
{
	inline constexpr int sa_func_reps = 3000;
	inline constexpr float sa_func_temp = 25;
	inline constexpr float sa_func_pow_mult = 20;

	template<typename FuncsDataRealTypeIn, typename OrderDataRealTypeIn>
	struct SAFunctionOrderSettings : public SimulatedAnnealingSettings<SAFunctionOrderSettings<FuncsDataRealTypeIn, OrderDataRealTypeIn>>
	{
		//idxs in orig funcs, use funcs_perm to get current idxs
		struct Delta {
			int origIdx1;
			int origIdx2;
		};

		using temp_t = float;
		using cost_t = double;
		using reps_t = int;

		//using function = function;
		std::mt19937 gen;
		int udistr_res;
		std::uniform_int_distribution<int> swapUnifDistr;

		using FuncsDataType = std::remove_cvref_t<FuncsDataRealTypeIn>;
		using OrderDataType = std::remove_cvref_t<OrderDataRealTypeIn>;
		using UnusedType = int;

		OrderDataType ordersData;
		FuncsDataType funcsData;
#define fdata funcsData

		static constexpr int funcCount = FuncsDataType::count;


		//use this before sorting so that for loop indexing matches func ID
		//first value is the ID of the function that will be swapped w/ another
		//second value is num of following func IDs, this value is taking the place of self ID before it
		//following values are IDs of functions this current one can be swapped with
#define delta_fid 0
#define delta_fswaps_num 1
		int delta_swap_mat_size{}; //number of funcs/rows in mat below, these funcs are swappable
		std::array<std::array<int, funcCount + 1>, funcCount> delta_swap_mat{}; //orig indexing

		std::array<int, funcCount> funcs_perm{}; //[x] represents the IDX in the FUNCS array of func with ID == x

		std::array<std::array<bool, funcCount>, funcCount> fswap_mat{}; //orig indexing
		std::array<std::array<bool, funcCount>, funcCount> fcmp_mat{};  // < comparison, orig indexing

		//returns f1 < f2
		std::pair<bool, bool> FuncOrdersCmp(const int fidx1, const int fidx2) {
			if (fidx1 == fidx2)
				return { true, true };

			const span<int> fords1 = fdata.f_orders(fidx1);
			const span<int> fords2 = fdata.f_orders(fidx2);

			//different functions but same order symbols
			if (fords1.len == fords2.len) {
				bool same = true;
				for (int i = 0; i < fords1.len; ++i)
					if (fords1[i] != fords2[i]) {
						same = false;
						break;
					}
				if (same)
					return { true, true };
			}

			bool smaller1 = true;
			for (int i = 0; i < fords1.len; ++i)
				for (int j = 0; j < fords2.len; ++j)
					if (!ordersData.cmp_mat[fords1[i]][fords2[j]]) {
						smaller1 = false;
						j = fords2.len;
						i = fords1.len;
					}

			bool smaller2 = true;
			for (int i = 0; i < fords2.len; ++i)
				for (int j = 0; j < fords1.len; ++j)
					if (!ordersData.cmp_mat[fords2[i]][fords1[j]]) {
						smaller2 = false;
						j = fords1.len;
						i = fords2.len;
					}

			//unlike orders, both functions can be wrongfully !smaller than eachother
			if (!smaller1 && !smaller2)
				throw;

			return { smaller1, smaller2 };
		}

		UnusedType TransformInput(const auto& ord_funcs_pair) {
			//this->funcs = std::forward<decltype(functions)>(functions);
			ordersData = ord_funcs_pair.first;
			funcsData = ord_funcs_pair.second;

			for (int i = 0; i < funcCount; ++i) {
				span<int> vec = funcsData.f_params(i);
				std::sort(vec.data, vec.data + vec.len);
			}

			//ver1
			for (int i = 0; i < funcCount; ++i)
				for (int j = 0; j < funcCount; ++j) {
					const std::pair<bool, bool> cmp = FuncOrdersCmp(i, j);
					fcmp_mat[i][j] = cmp.first;
					fswap_mat[i][j] = cmp.first && cmp.second;
				}

			//ver2
			//for (int i = 0; i < funcCount; ++i)
			//	for (int j = i; j < funcCount; ++j) {
			//		const std::pair<bool, bool> cmp = FuncOrdersCmp(funcsData, i, j);
			//		fcmp_mat[i][j] = cmp.first;
			//		fcmp_mat[j][i] = cmp.second;
			//		fswap_mat[j][i] = (fswap_mat[i][j] = cmp.first && cmp.second);
			//	}

#define delta_swap_row_offset 2
//col_pos == row_idx
//row_pos == col_idx
//delta_swap_row_offset = col_idx_offset
			int col_pos = 0;
			for (int i = 0; i < funcCount; ++i) {
				int row_pos = delta_swap_row_offset;
				for (int j = 0; j < i; ++j) {
					if (fswap_mat[i][j])
						delta_swap_mat[col_pos][row_pos++] = j;
				}
				//skip i == j
				for (int j = i + 1; j < funcCount; ++j) {
					if (fswap_mat[i][j])
						delta_swap_mat[col_pos][row_pos++] = j;
				}

				const int row_size = row_pos - delta_swap_row_offset;
				//if unswappable, exclude
				if (row_size == 0) {
					--i;
					continue;
				}

				delta_swap_mat[col_pos][0] = i; //func ID
				delta_swap_mat[col_pos][1] = row_size;
				++col_pos;
			}
			delta_swap_mat_size = col_pos;


			//sorting, ID == idx at this line
			int pos = 0;
			for (int i = pos; i < funcCount; ++i) {
				bool isMin = true;
				for (int j = pos; j < funcCount; ++j) {
					//check if smaller than all
					if (!fcmp_mat[funcsData.funcs[i].ID][funcsData.funcs[j].ID]) {
						isMin = false;
						break;
					}
				}
				if (isMin) {//swap with pos if min
					std::swap(funcsData.funcs[pos], funcsData.funcs[i]);

					//the MIN that was in I is now in POS so we change funcs_perm[funcsData.funcs[pos].ID] 
					funcs_perm[funcsData.funcs[pos].ID] = pos; //func with ID == i is on position pos in funcs
					//funcs_perm[funcsData.funcs[i].ID] = i; //func with ID == i is on position pos in funcs

					pos++;
					i = pos - 1; //to negate i++
				}
			}

			//sanity check
			//for (int i = 0; i < funcCount; ++i) {
			//	int pos = funcs_perm[i];
			//	if (funcsData.funcs[pos].ID != i) {
			//		meta::compiler.print(__concatenate(funcsData.funcs[pos].ID, " ", i));
			//		meta::compiler.print("BBBBBBBBBBBB");
			//		throw;
			//	}
			//	else
			//		;// meta::compiler.print(__concatenate(pos, " ", i));
			//}

			swapUnifDistr = std::uniform_int_distribution<int>(0, funcCount - 1);
			udistr_res = swapUnifDistr(gen); //roll out a value

			return {};
		}

		UnusedType InitialSolution(UnusedType) {
			return 0;
		}

		bool CanBeOptimized(UnusedType) {
			return funcCount > 1 && delta_swap_mat_size > 0;
		}

		cost_t GetCostAtMid(const int fidx) {
			return Cost(fidx - 1, 3);
		}

		cost_t GetCostAtLeft(const int fidx) {
			return Cost(fidx - 1, 2);
		}

		cost_t GetCostAtRight(const int fidx) {
			return Cost(fidx, 2);
		}

		cost_t Cost(const int fstart, const int fnum) {
			cost_t cost = 0.0f;
			for (int i = fstart; i < fstart + fnum - 1; ++i)
				cost += JaccardIndexOfSortedSets(fdata.f_params(fdata.funcs[i]), fdata.f_params(fdata.funcs[i + 1]));

			return cost;
		}

		cost_t Cost(const UnusedType) {
			cost_t cost = 0.0f;
			for (int i = 0; i < funcCount - 1; ++i)
				cost += JaccardIndexOfSortedSets(fdata.f_params(fdata.funcs[i]), fdata.f_params(fdata.funcs[i + 1]));

			return cost;
		}


		//fix what?
		cost_t CostAndApply(UnusedType, cost_t cost, const Delta& delta) {

			using StructType = std::remove_pointer_t<decltype(this)>;
			auto& fidx1 = funcs_perm[delta.origIdx1];
			auto& fidx2 = funcs_perm[delta.origIdx2];

			if (fidx1 != 0 && (fidx2 != funcCount - 1))
			{
				const cost_t oldCostDelta = GetCostAtMid(fidx1) + GetCostAtMid(fidx2);
				cost -= oldCostDelta;

				std::swap(fdata.funcs[fidx1], fdata.funcs[fidx2]);

				const cost_t newCostDelta = GetCostAtMid(fidx1) + GetCostAtMid(fidx2);
				cost += newCostDelta;

				std::swap(fidx1, fidx2);
			}
			else
			{
				auto El1Func = &StructType::GetCostAtMid;
				auto El2Func = &StructType::GetCostAtMid;
				if (fidx1 == 0)
					El1Func = &StructType::GetCostAtRight;
				if (fidx2 == funcCount - 1)
					El2Func = &StructType::GetCostAtLeft;

				const cost_t oldCostDelta = (this->*El1Func)(fidx1) + (this->*El2Func)(fidx2);
				cost -= oldCostDelta;

				std::swap(fdata.funcs[fidx1], fdata.funcs[fidx2]);

				const cost_t newCostDelta = (this->*El1Func)(fidx1) + (this->*El2Func)(fidx2);
				cost += newCostDelta;

				std::swap(fidx1, fidx2);
				//don't revert the swap because it's done in SA when calling
				//calling RevertNeighbourDelta if we don't want to keep the change
				//std::swap(dfuncs.funcs[fidx1], dfuncs.funcs[fidx2]);
				//std::swap(fidx1, fidx2);
			}


			return cost;
		}

		Delta NeighbourDelta(const UnusedType) {
			bool validSwap = false;
			int origIdx1, origIdx2;
			while (!validSwap) {
				//pick functions in the original indexing
				auto minVal = 0;
				auto maxVal = delta_swap_mat_size - 1;
				swapUnifDistr.param(typename std::uniform_int<int>::param_type{ minVal, maxVal });
				const auto row = swapUnifDistr(gen);
				origIdx1 = delta_swap_mat[row][delta_fid];

				minVal = delta_fswaps_num + 1;
				maxVal = delta_swap_mat[row][delta_fswaps_num] - 1;//todo check
				swapUnifDistr.param(typename std::uniform_int<int>::param_type{ minVal, maxVal });
				const int col = swapUnifDistr(gen);
				origIdx2 = delta_swap_mat[row][col];

				//swap because array limit cases in CostAndApply
				if (funcs_perm[origIdx2] < funcs_perm[origIdx1])
					std::swap(origIdx1, origIdx2);

				//if (origIdx2 == origIdx1)
				//	throw;
				//if (funcs_perm[origIdx2] == funcs_perm[origIdx1])
				//	throw;

				int minIdx = funcs_perm[origIdx1];
				int maxIdx = funcs_perm[origIdx2];

				validSwap = true;
				if (maxIdx - minIdx == 1)
					continue;

				//do incremental checks to maxIdx target, if fail along the way we settle for worse
				int cursor = minIdx + 1;
				for (; cursor < maxIdx; ++cursor)
					if (!fcmp_mat[fdata.funcs[cursor].ID][fdata.funcs[minIdx].ID])//looking in multiple rows
						break;
				if (cursor == minIdx + 1) { //didn't move cursor, can't swap w/ anything
					validSwap = false;
					continue;
				}
				cursor--;

				maxIdx = cursor;
				validSwap = false;
				while (!validSwap) {
					validSwap = true;
					int j = maxIdx - 1;
					for (; j > minIdx; --j)
						if (!fcmp_mat[fdata.funcs[maxIdx].ID][fdata.funcs[j].ID]) {//looking in one row
							validSwap = false;
							maxIdx--;
							break;
						}
					if (maxIdx == minIdx) {
						validSwap = false;
						break;
					}
				}

				origIdx2 = fdata.funcs[maxIdx].ID;
			}

			return { origIdx1, origIdx2 };
		}

		void ApplyNeighbourDelta(UnusedType, const Delta& delta) {
			auto& fidx1 = funcs_perm[delta.origIdx1];
			auto& fidx2 = funcs_perm[delta.origIdx2];

			std::swap(fdata.funcs[fidx1], fdata.funcs[fidx2]);
			std::swap(fidx1, fidx2);

			//for (int i = 0; i < funcCount; ++i) {
			//	int pos = funcs_perm[i];
			//	if (fdata.funcs[pos].ID != i) {
			//		meta::compiler.print(__concatenate(fdata.funcs[pos].ID, " ", i));
			//		throw;
			//	}
			//}
		}

		void RevertNeighbourDelta(UnusedType, const Delta& delta) {
			ApplyNeighbourDelta({}, delta);
		}

		float Probability(const cost_t neg_cost_diff, const temp_t temp) {
			//neg_cost_diff is negative
			return std::exp(neg_cost_diff / temp * sa_func_pow_mult);
			//return gcem::exp<float>(neg_cost_diff / temp * sa_func_pow_mult);
		}

		void ChangeSchedules(const int k, temp_t& temp, reps_t& reps) {
			/*temp -= 0.1f;
			reps += 20;*/
			temp -= 0.05f;
			reps += 5;
		}

		void InitSchedules(temp_t& temp, reps_t& reps) {
			temp = sa_func_temp;
			reps = sa_func_reps;
		}

		bool StopCheck(const temp_t& temp) {
			const float val = (funcCount) / temp * sa_func_pow_mult; //maximum value supported by the approx
			if (val < -700.0f || val > 700.0f)
				return true;
			return temp <= 0;
		}

		auto TransformOutput(const UnusedType) {
			for (int i = 0; i < funcCount; ++i) {
				bool isMin = true;
				for (int j = i; j < funcCount; ++j) {
					//check if smaller than all
					if (!fcmp_mat[funcsData.funcs[i].ID][funcsData.funcs[j].ID]) {
						isMin = false;
						break;
					}
				}
				if (!isMin)
					throw;
			}

			return fdata;
		}
	};

} // namespace Meta