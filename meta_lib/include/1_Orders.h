#pragma once

#include "misc/meta_utils.h"
#include <vector>
#include <algorithm>

namespace Meta
{
	template<size_t orderCount, size_t rulesStorageSize>
	struct OrdersDataReal {
		using order_t = int;

		std::array<order_t, rulesStorageSize> rule_storage{}; //sz1 el11... sz2 el21...  ....
		std::array<int, orderCount> rules{};

		static constexpr int count = orderCount;

		constexpr auto rule_size(int idx) const {
			return at_storage(rules[idx]);
		}
		constexpr auto& rule_size(int idx) {
			return at_storage(rules[idx]);
		}

		constexpr auto rule_data(int idx, int j) const {
			return at_storage(rules[idx] + j + 1);
		}
		constexpr auto& rule_data(int idx, int j) {
			return at_storage(rules[idx] + j + 1);
		}
	public:
		constexpr auto at_storage(int idx) const {
			return rule_storage[idx];
		}
		constexpr auto& at_storage(int idx) {
			return rule_storage[idx];
		}
	};

	struct OrdersNameID{
		const char* name;
		int ID;

		static constexpr bool name_cmp(const OrdersNameID& lsh, const OrdersNameID& rsh) {
			return const_strcmp(lsh.name, rsh.name) < 0;
		}
	};

	template<size_t orderCount>
	struct OrdersDataImag {
		std::array<meta::info  , orderCount> metas{};
		static constexpr int count = orderCount;
	};

	template<typename T1, typename T2>
	struct OrdersDataRI {
		T1 real;
		T2 imag;
	};

	template<size_t orderCount>
	struct OrdersCmpSwapMats {
		std::array<std::array<bool, orderCount>, orderCount> swap{};
		std::array<std::array<int, orderCount>, orderCount> cmp{};// < 
	};

	template<auto namespaceMeta>
	consteval size_t CalcOrderRulesStorageSize() {
		size_t sum = 0;
		for (meta::info orderMeta : meta::members_of(namespaceMeta, meta::is_class)) {
			sum += size(meta::base_spec_range(orderMeta));
		}
		return sum;
	}

	//returns e1 < e2, e2 < e1
	std::pair<char, char> OrdersCmp(const auto& expanded_bases, int primary, int secondary) {
		if (primary == secondary)
			return { 3, 3 };

		//std::vector<order_t>
		auto& prim_exp_rules = expanded_bases[primary];
		auto& sec_exp_rules = expanded_bases[secondary];

		//primary smaller than secondary
		for (int j = 0; j < sec_exp_rules.size(); ++j)
			if (primary == sec_exp_rules[j])
				return { 1, 0 };

		//secondary smaller than primary
		for (int i = 0; i < prim_exp_rules.size(); ++i)
			if (prim_exp_rules[i] == secondary)
				return { 0, 1 };

		return { 1, 1 };
	}

	bool And(const std::pair<char, char>& pair) {
		return pair.first && pair.second;
	}

	// Add all bases recursively
	void AddBasesFrom(const auto& ordersReal, auto& vec, int idx) {
		// For each base/rule add their bases/rules
		for (int i = 0; i < ordersReal.rule_size(idx); ++i) {
			const int base_idx = ordersReal.rule_data(idx, i);

			vec.push_back(base_idx); // Add current
			AddBasesFrom(ordersReal, vec, base_idx); // Add subrules
		}
	}

	void ComputeOrdersCmp(auto& ordersCmpSwapMats, const auto& ordersReal) {
		using order_t = typename std::remove_cvref_t<decltype(ordersReal)>::order_t;
		constexpr const int orderCount = ordersReal.count;

		using expanded_bases_t = std::array<std::vector<order_t>, orderCount>; //O(N^2)
		expanded_bases_t expanded_bases;

		// Store rules in a flat vector instead of tree
		for (int i = 0; i < orderCount; ++i) {
			expanded_bases[i].push_back(i);
			AddBasesFrom(ordersReal, expanded_bases[i], i);
		}

		// Compute cmp and swap matrices
		for (int i = 0; i < orderCount; ++i) {
			for (int j = 0; j < i; ++j) {
				const auto& cmp_pair = OrdersCmp(expanded_bases, i, j);
				ordersCmpSwapMats.swap[i][j] = And(cmp_pair); // i < j && j < i aka canSwap?
				ordersCmpSwapMats.cmp[i][j] = cmp_pair.first; // if i < j
			}

			ordersCmpSwapMats.swap[i][i] = 1; // i < i
			ordersCmpSwapMats.cmp[i][i] = 3; // ==

			for (int j = i + 1; j < orderCount; ++j) {
				const auto& cmp_pair = OrdersCmp(expanded_bases, i, j);
				ordersCmpSwapMats.swap[i][j] = And(cmp_pair);
				ordersCmpSwapMats.cmp[i][j] = 1; //because i was declared before j
			}
		}
	}

	auto CreateOrdersCmpSwapMats(const auto& ordersReal) {
		constexpr const int orderCount = ordersReal.count;

		OrdersCmpSwapMats<orderCount> ordersCmpSwapMats;
		ComputeOrdersCmp(ordersCmpSwapMats, ordersReal);

		return ordersCmpSwapMats;
	}

	template<auto orderNamespaceMeta>
	consteval auto CreateOrdersData() {
		// Order symbols are stored in struct/class names
		// Count all struct/class declarations in namespace
		constexpr auto orderMetaRange = meta::members_of(orderNamespaceMeta, meta::is_class);
		constexpr size_t orderCount = size(orderMetaRange);

		// Order rules are based on inheritance
		// Count all struct/class declarations in namespace + all their bases(which define the rules)
		constexpr size_t orderRulesTotal = CalcOrderRulesStorageSize<orderNamespaceMeta>();
		constexpr size_t ordersStorageSize = orderCount + orderRulesTotal;

		OrdersDataReal<orderCount, ordersStorageSize> ordersReal{};
		OrdersDataImag<orderCount> ordersImag{};
		std::array<OrdersNameID, orderCount> nameID{};

		// Gather metas and names of all orders
		for (int orderIdx = 0; meta::info orderMeta : orderMetaRange) {
			ordersImag.metas[orderIdx] = orderMeta;

			nameID[orderIdx].name = meta::name_of(orderMeta);
			nameID[orderIdx].ID = orderIdx;
			++orderIdx;
		}
		// Sort by name for rule binary search
		std::sort(nameID.begin(), nameID.end(), OrdersNameID::name_cmp);
		meta::compiler.print("Sorting done");

		int rule_storage_pos = 0;
		for (int orderIdx = 0; meta::info orderMeta : orderMetaRange) {
			auto ruleMetaRange = meta::base_spec_range(orderMeta);
			auto ruleMetaRangeSize = size(ruleMetaRange);
			meta::compiler.print("one order done");

			// Prepare rule storage
			ordersReal.rules       [orderIdx]         =	rule_storage_pos;
			ordersReal.rule_storage[rule_storage_pos] = ruleMetaRangeSize; // Per symbol, first elem is count of rules followed by rules
			rule_storage_pos += ruleMetaRangeSize + 1; // Advance by num of rules/bases for preparing next iteration

			// Fill rule storage
			// Go through each each rule/base of a symbol
			int currRule = 0; // Index of rule/base for current order symbol
			for (auto rule : ruleMetaRange) {
				// Binary search rule by name in nameID
				OrdersNameID ruleName{ meta::name_of(rule), -1 };
				auto iter = std::lower_bound(nameID.begin(), nameID.end(), ruleName, OrdersNameID::name_cmp);
				auto ruleIdx = iter->ID; // Namespace order

				ordersReal.rule_data(orderIdx, currRule) = ruleIdx;
				++currRule;
			}

			++orderIdx;
		}
		meta::compiler.print("All orders done");

		return OrdersDataRI<decltype(ordersReal), decltype(ordersImag)>{ {}, {} };
	}

} // namespace Meta