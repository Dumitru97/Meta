#pragma once
#include "../1_Orders.h"
#include "../3_Functions.h"
#include "SimulatedAnnealing.h"
#include "CostFunctions.h"
#include <limits>
#include <vector>
#include <algorithm>
#include <optional>

namespace Meta
{
	template<typename OrdersDataRealType, typename FuncsDataRealType, typename OrdersCmpSwapMatsType, typename FuncsCmpSwapMatsType>
	auto SAFunctionOrderOP(const auto& input);

	namespace SAFunctionOrder {

		struct SAParams {
			int reps;
			float temp;
			int reps_increment;
			float temp_decrement;
			float pow_mult;
		};

		template<typename AdditionalFunctionInfo>
		bool operator<(const function<AdditionalFunctionInfo>& lhs,
					   const function<AdditionalFunctionInfo>& rhs)
		{
			using orderNamespaceHelper = typename AdditionalFunctionInfo::orderNamespaceHelper;
			using funcNamespaceHelper = typename AdditionalFunctionInfo::funcNamespaceHelper;
			using FuncsCmpSwapMatsType = FuncsCmpSwapMats<AdditionalFunctionInfo::funcCount, orderNamespaceHelper, funcNamespaceHelper>;
			return FuncsCmpSwapMatsType::cmp[lhs.ID][rhs.ID];
		}

		template<typename AdditionalFunctionInfo>
		bool operator<=>(const function<AdditionalFunctionInfo>& lhs,
						 const function<AdditionalFunctionInfo>& rhs)
		{
			using orderNamespaceHelper = typename AdditionalFunctionInfo::orderNamespaceHelper;
			using funcNamespaceHelper = typename AdditionalFunctionInfo::funcNamespaceHelper;
			using FuncsCmpSwapMatsType = FuncsCmpSwapMats<AdditionalFunctionInfo::funcCount, orderNamespaceHelper, funcNamespaceHelper>;
			return FuncsCmpSwapMatsType::swap[lhs.ID][rhs.ID];
		}

		template<typename OrdersDataRealTypeIn, typename FuncsDataRealTypeIn, typename OrdersCmpSwapMatsTypeIn, typename FuncsCmpSwapMatsTypeIn>
		struct SASettings : public SimulatedAnnealingSettings<SASettings<OrdersDataRealTypeIn, FuncsDataRealTypeIn, OrdersCmpSwapMatsTypeIn, FuncsCmpSwapMatsTypeIn>>
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

			static constexpr size_t delta_func_ID = 0;
			static constexpr size_t delta_fswaps_count = 1;

			// Member variables
		public:
			SAParams sa_params = { .reps = 100, .temp = 25, .reps_increment = 250, .temp_decrement = 0.08f, .pow_mult = 20 };

			//OrdersDataType odata;
			FuncsDataType fdata;
			decltype(fdata.funcs)& funcs = fdata.funcs;
			FuncsCmpSwapMatsType fmats;
			//OrdersCmpSwapMatsType omats;
			static constexpr int funcCount = FuncsDataType::count;

			// Holds for every function all the functions that it can be swapped with. Used in NeighbourDelta().
			std::array<std::array<int, funcCount + 2 - 1>, funcCount> delta_swap_mat{};	// +2(func id and row size) -1(excluding self swapping)
			int delta_swappable_func_count{};											// Number of swappable functions in delta_swap_mat

			std::array<int, funcCount> funcs_perm{}; // Use function ID to get the index in fdata after optimizing

			std::mt19937 gen;
			std::uniform_int_distribution<int> swapUnifDistr;

			FuncsDataType initFuncsData;
			float initcost;
			//Member functions for the CRTP interface
		public:
			UnusedType TransformInput(const auto& ord_funcs_data_and_mat_tuple_and_saparams) {
				// Copy into member variables
				//odata = std::get<0>(ord_funcs_data_and_mat_tuple_and_saparams);
				fdata = std::get<1>(ord_funcs_data_and_mat_tuple_and_saparams);
				//omats = std::get<2>(ord_funcs_data_and_mat_tuple_and_saparams);
				fmats = std::get<3>(ord_funcs_data_and_mat_tuple_and_saparams);
				auto optionalParams = std::get<4>(ord_funcs_data_and_mat_tuple_and_saparams);
				
				initFuncsData = fdata;
				if (optionalParams)
					sa_params = optionalParams.value();

				// Sort parameters for Jaccard index calculation
				for (int i = 0; i < funcCount; ++i) {
					span<int> vec = fdata.f_params(i);
					std::sort(vec.data, vec.data + vec.len);
				}

				// Check if ordering is valid and initialize funcs_perm
				bool isValidOrdering = true;
				for (int i = 0; i < funcCount; ++i) {
					for (int j = i; j < funcCount; ++j) {
						// Check if smaller than all
						if (!(funcs[i] < funcs[j])) {
							isValidOrdering = false;
							std::cout << "Invalid ordering. Func " << funcs[i].ID << " bigger than " << funcs[j].ID << "\n";
							break;
						}
					}

					if (!isValidOrdering)
						break;

					funcs_perm[funcs[i].ID] = i;
				}

				// Compute delta_swap_mat
				constexpr int delta_swap_row_offset = 2;
				int col_pos = 0;
				for (int i = 0; i < funcCount; ++i) {
					int row_pos = delta_swap_row_offset;
					for (int j = 0; j < i; ++j) {
						if (fmats.swap[i][j])
							delta_swap_mat[col_pos][row_pos++] = j;
					}
					for (int j = i + 1; j < funcCount; ++j) {
						if (fmats.swap[i][j])
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

				if (!isValidOrdering) {
					std::cout << "Reordering function into a valid ordering" << "\n";

					// Sorting the functions into a valid ordering, ID == idx will not true anymore
					// Computing funcs_perm
					int minIdx = 0; // Idx of min function to be found
					for (int i = minIdx; i < funcCount; ++i) {
						bool isMin = true;

						// Check if smaller than all
						for (int j = minIdx; j < funcCount; ++j) {
							if (!(funcs[i] < funcs[j])) {
								isMin = false;
								break;
							}
						}

						if (isMin) { // Swap with minIdx if min, otherwise 'i' advances to look for a min func
							std::swap(funcs[minIdx], funcs[i]);
							funcs_perm[funcs[minIdx].ID] = minIdx;

							// Found a min and placed it in minIdx, now 'i' is reset
							minIdx++;
							i = minIdx - 1; // To negate i++
						}
					}
				}

				// Compute initial cost for difference
				initcost = Cost(UnusedType{});

				// Sanity check funcs_perm
				for (int i = 0; i < funcCount; ++i) {
					int pos = funcs_perm[i];
					if (funcs[pos].ID != i)
						throw;
				}

				//std::cout << "Valid input ordering:\n";
				//for (int i = 0; i < funcCount; ++i)
				//	std::cout << funcs[i].ID << " ";
				//std::cout << "\n";

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
					const auto& currFunc = fdata.f_params(funcs[i]);
					const auto& nextFunc = fdata.f_params(funcs[i + 1]);
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

					std::swap(funcs[fidx1], funcs[fidx2]);

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

					std::swap(funcs[fidx1], funcs[fidx2]);

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
						if (!(funcs[cursor] < funcs[minIdx])) // Looking in multiple rows to check if swap is valid
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
							if (!(funcs[maxIdx] < funcs[j])) { // Looking in one row to check if swap is valid
								validSwap = false;
								maxIdx--;
								break;
							}
						if (maxIdx == minIdx) {
							validSwap = false;
							break;
						}
					}

					origIdx2 = funcs[maxIdx].ID;
				}

				return { origIdx1, origIdx2 };
			}

			void ApplyNeighbourDelta(UnusedType, const Delta& delta) {
				auto& fidx1 = funcs_perm[delta.origIdx1];
				auto& fidx2 = funcs_perm[delta.origIdx2];

				std::swap(funcs[fidx1], funcs[fidx2]);
				std::swap(fidx1, fidx2);
			}

			void RevertNeighbourDelta(UnusedType, const Delta& delta) {
				ApplyNeighbourDelta({}, delta);
			}

			float Probability(const cost_t neg_cost_diff, const temp_t temp) {
				return std::exp(neg_cost_diff / temp * sa_params.pow_mult);
			}

			void ChangeSchedules(const int k, temp_t& temp, reps_t& reps) {
				temp -= sa_params.temp_decrement;
				reps += sa_params.reps_increment;
			}

			void InitSchedules(temp_t& temp, reps_t& reps) {
				temp = sa_params.temp;
				reps = sa_params.reps;
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
						if (!(funcs[i] < funcs[j])) {
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
					if (funcs[pos].ID != i)
						throw;
				}

				auto cost = Cost(UnusedType{});
				auto costdiff = initcost - cost;
				std::cout << std::format("SimulatedAnnealing - Valid input cost: {}, Output cost: {}, Diff: {}\n", initcost, cost, costdiff);

				if (costdiff < -1E-3f) {
					std::cout << "Cost increase. Using returning input instead of output order." << "\n";
					return initFuncsData;
				}

				//std::cout << "New ordering:\n";
				//for (int i = 0; i < funcCount; ++i)
				//	std::cout << funcs[i].ID << " ";
				//std::cout << "\n";

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
					const auto& currFunc = fdata.f_params(funcs[i]);
					const auto& nextFunc = fdata.f_params(funcs[i + 1]);
					cost += JaccardIndexOfSortedSets(currFunc, nextFunc);
				}

				return cost;
			}
		};

	} // namespace SAFunctionOrder

	template<typename OrdersDataRealType, typename FuncsDataRealType, typename OrdersCmpSwapMatsType, typename FuncsCmpSwapMatsType>
	auto SAFunctionOrderOP(const auto& input) {
		auto newFuncsDataReal = Meta::SimulatedAnnealing<
			SAFunctionOrder::SASettings<OrdersDataRealType, FuncsDataRealType,
			OrdersCmpSwapMatsType, FuncsCmpSwapMatsType>
		>(input);
		return newFuncsDataReal;
	}

} // namespace Meta