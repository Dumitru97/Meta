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
	extern const int sa_func_reps;
	extern const float sa_func_temp;
	extern const int sa_func_reps_increment;
	extern const float sa_func_temp_decrement;
	extern const float sa_func_pow_mult;

	template<typename OrdersDataRealTypeIn, typename FuncsDataRealTypeIn, typename OrdersCmpSwapMatsTypeIn, typename FuncsCmpSwapMatsTypeIn>
	struct SAFunctionOrderSettings : public SimulatedAnnealingSettings<SAFunctionOrderSettings<OrdersDataRealTypeIn, FuncsDataRealTypeIn, OrdersCmpSwapMatsTypeIn, FuncsCmpSwapMatsTypeIn>>
	{
		// Structs and typedefs
	public:
		struct Delta { // Func idxs/ID in original order, use funcs_perm to get current idxs
			int origIdx1;
			int origIdx2;
		};

		using temp_t = float;
		using cost_t = double;
		using reps_t = int;

		using OrdersDataType = std::remove_cvref_t<OrdersDataRealTypeIn>;
		using FuncsDataType = std::remove_cvref_t<FuncsDataRealTypeIn>;
		using OrdersCmpSwapMatsType = std::remove_cvref_t<OrdersCmpSwapMatsTypeIn>;
		using FuncsCmpSwapMatsType = std::remove_cvref_t<FuncsCmpSwapMatsTypeIn>;
		using UnusedType = int;

#define fdata funcsData
#define delta_func_ID 0
#define delta_fswaps_count 1

		// Member variables
	public:
		OrdersDataType ordersData;
		FuncsDataType funcsData;
		const FuncsCmpSwapMatsType* funcsCmpSwapMats;
		const OrdersCmpSwapMatsType* ordersCmpSwapMats;
		static constexpr int funcCount = FuncsDataType::count;

#define fswap_mat funcsCmpSwapMats->swap
#define fcmp_mat  funcsCmpSwapMats->cmp

		// Holds for every function all the functions that it can be swapped with. Used in NeighbourDelta().
		std::array<std::array<int, funcCount + 2 - 1>, funcCount> delta_swap_mat{};	// +2(func id and row size) -1(excluding self swapping)
		int delta_swappable_func_count{};											// Number of swappable functions in delta_swap_mat

		std::array<int, funcCount> funcs_perm{}; // Use function ID to get the index in funcsData after optimizing

		std::mt19937 gen;
		std::uniform_int_distribution<int> swapUnifDistr;

		//Member functions for the CRTP interface
	public:
		UnusedType TransformInput(const auto& ord_funcs_data_and_mat_tuple) {
			// Copy into member variables
			ordersData = std::get<0>(ord_funcs_data_and_mat_tuple);
			funcsData = std::get<1>(ord_funcs_data_and_mat_tuple);
			ordersCmpSwapMats = &std::get<2>(ord_funcs_data_and_mat_tuple);
			funcsCmpSwapMats = &std::get<3>(ord_funcs_data_and_mat_tuple);

			// Sort parameters for Jaccard index calculation
			for (int i = 0; i < funcCount; ++i) {
				span<int> vec = funcsData.f_params(i);
				std::sort(vec.data, vec.data + vec.len);
			}

			// Compute delta_swap_mat
			constexpr int delta_swap_row_offset = 2;
			int col_pos = 0;
			for (int i = 0; i < funcCount; ++i) {
				int row_pos = delta_swap_row_offset;
				for (int j = 0; j < i; ++j) {
					if (fswap_mat[i][j])
						delta_swap_mat[col_pos][row_pos++] = j;
				}
				for (int j = i + 1; j < funcCount; ++j) {
					if (fswap_mat[i][j])
						delta_swap_mat[col_pos][row_pos++] = j;
				}

				const int row_size = row_pos - delta_swap_row_offset;
				// If unswappable, do not include in delta_swap_mat
				if (row_size == 0)
					continue;

				delta_swap_mat[col_pos][0] = i; // Func ID
				delta_swap_mat[col_pos][1] = row_size;
				++col_pos;
			}
			delta_swappable_func_count = col_pos;

			// Sorting the functions into a valid ordering, ID == idx will not true anymore
			// Computing funcs_perm
			int pos = 0; // Position of min function to be found
			for (int i = pos; i < funcCount; ++i) {
				bool isMin = true;

				// Check if smaller than all
				for (int j = pos; j < funcCount; ++j) {
					if (!fcmp_mat[fdata.funcs[i].ID][fdata.funcs[j].ID]) {
						isMin = false;
						break;
					}
				}

				if (isMin) { // Swap with pos if min, otherwise 'i' advances to look for a min func
					std::swap(fdata.funcs[pos], fdata.funcs[i]);
					funcs_perm[fdata.funcs[pos].ID] = pos;

					// Found a min and placed it in pos, now 'i' is reset
					pos++;
					i = pos - 1; // To negate i++
				}
			}

			// Sanity check funcs_perm
			for (int i = 0; i < funcCount; ++i) {
				int pos = funcs_perm[i];
				if (funcsData.funcs[pos].ID != i)
					throw;
			}

			std::cout << "Old ordering:\n";
			for (int i = 0; i < funcCount; ++i)
				std::cout << funcsData.funcs[i].ID << " ";
			std::cout << "\n";

			return {};
		}

		UnusedType InitialSolution(UnusedType) {
			return {};
		}

		bool CanBeOptimized(UnusedType) {
			return funcCount > 1 && delta_swappable_func_count > 0;
		}

		cost_t Cost(const UnusedType) {
			cost_t cost = 0.0f;
			for (int i = 0; i < funcCount - 1; ++i) {
				const auto& currFunc = fdata.f_params(fdata.funcs[i]);
				const auto& nextFunc = fdata.f_params(fdata.funcs[i + 1]);
				cost += JaccardIndexOfSortedSets(currFunc, nextFunc);
			}

			return cost;
		}

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
			}

			// Don't revert the swap because it's done in SA when calling RevertNeighbourDelta
			return cost;
		}

		Delta NeighbourDelta(const UnusedType) {
			bool validSwap = false;
			int origIdx1, origIdx2;
			while (!validSwap) {
				// Random a swappable function
				auto minVal = 0;
				auto maxVal = delta_swappable_func_count - 1;
				swapUnifDistr.param(typename std::uniform_int<int>::param_type{ minVal, maxVal });
				const auto row = swapUnifDistr(gen);
				origIdx1 = delta_swap_mat[row][delta_func_ID];

				// Random a function to swap with
				minVal = delta_fswaps_count + 1;
				maxVal = delta_swap_mat[row][delta_fswaps_count] - 1;
				swapUnifDistr.param(typename std::uniform_int<int>::param_type{ minVal, maxVal });
				const int col = swapUnifDistr(gen);
				origIdx2 = delta_swap_mat[row][col];

				// Order because array limit cases in CostAndApply
				if (funcs_perm[origIdx2] < funcs_perm[origIdx1])
					std::swap(origIdx1, origIdx2);

				//if (origIdx2 == origIdx1)
				//	throw;
				//if (funcs_perm[origIdx2] == funcs_perm[origIdx1])
				//	throw;

				// Since swapping two distant functions can break ordering for the functions between them we 
				// do incremental checks(with cursor) up to maxIdx. If failing along the way we settle for a function in between
				int minIdx = funcs_perm[origIdx1];
				int maxIdx = funcs_perm[origIdx2];

				validSwap = true;
				if (maxIdx - minIdx == 1)
					continue;

				int cursor = minIdx + 1;
				for (; cursor < maxIdx; ++cursor)
					if (!fcmp_mat[fdata.funcs[cursor].ID][fdata.funcs[minIdx].ID]) // Looking in multiple rows to check if swap is valid
						break;
				if (cursor == minIdx + 1) { // Didn't move cursor, can't swap with anything
					validSwap = false;
					continue;
				}
				cursor--;

				// Settled for initial maxIdx or a function in between
				// Now checking if that maxIdx/cursor function can be swapped in the other direction to minIdx
				maxIdx = cursor;
				validSwap = false;
				while (!validSwap) {
					validSwap = true;
					int j = maxIdx - 1;
					for (; j > minIdx; --j)
						if (!fcmp_mat[fdata.funcs[maxIdx].ID][fdata.funcs[j].ID]) { // Looking in one row to check if swap is valid
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
		}

		void RevertNeighbourDelta(UnusedType, const Delta& delta) {
			ApplyNeighbourDelta({}, delta);
		}

		float Probability(const cost_t neg_cost_diff, const temp_t temp) {
			return std::exp(neg_cost_diff / temp * sa_func_pow_mult);
		}

		void ChangeSchedules(const int k, temp_t& temp, reps_t& reps) {
			temp -= sa_func_temp_decrement;
			reps += sa_func_reps_increment;
		}

		void InitSchedules(temp_t& temp, reps_t& reps) {
			temp = sa_func_temp;
			reps = sa_func_reps;
		}

		bool StopCheck(const temp_t& temp) {
			return temp <= 0;
		}

		auto TransformOutput(const UnusedType) {
			// Cmp sanity check
			for (int i = 0; i < funcCount; ++i) {
				bool isMin = true;
				for (int j = i; j < funcCount; ++j) {
					// Check if smaller than all
					if (!fcmp_mat[fdata.funcs[i].ID][fdata.funcs[j].ID]) {
						isMin = false;
						break;
					}
				}
				if (!isMin)
					throw;
			}

			// Sanity check funcs_perm
			for (int i = 0; i < funcCount; ++i) {
				int pos = funcs_perm[i];
				if (fdata.funcs[pos].ID != i)
					throw;
			}

			std::cout << "New ordering:\n";
			for (int i = 0; i < funcCount; ++i)
				std::cout << funcsData.funcs[i].ID << " ";
			std::cout << "\n";

			return fdata;
		}

	private:
		cost_t GetCostAtMid(const int fidx) {
			return Cost(fidx - 1, 3);
		}

		cost_t GetCostAtLeft(const int fidx) {
			return Cost(fidx - 1, 2);
		}

		cost_t GetCostAtRight(const int fidx) {
			return Cost(fidx, 2);
		}

		cost_t Cost(const int fstart, const int fcount) {
			cost_t cost = 0.0f;
			for (int i = fstart; i < fstart + fcount - 1; ++i) {
				const auto& currFunc = fdata.f_params(fdata.funcs[i]);
				const auto& nextFunc = fdata.f_params(fdata.funcs[i + 1]);
				cost += JaccardIndexOfSortedSets(currFunc, nextFunc);
			}

			return cost;
		}
	};

} // namespace Meta