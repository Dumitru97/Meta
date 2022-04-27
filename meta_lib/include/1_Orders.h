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
			return rule_storage[rules[idx]];
		}
		constexpr auto& rule_size(int idx) {
			return rule_storage[rules[idx]];
		}

		constexpr auto rule_data(int idx, int j) const {
			return rule_storage[rules[idx] + j + 1];
		}
		constexpr auto& rule_data(int idx, int j) {
			return rule_storage[rules[idx] + j + 1];
		}
	};

	struct OrdersNameID {
		sv name;
		int ID;

		static constexpr bool name_cmp(const OrdersNameID& lsh, const OrdersNameID& rsh) {
			return const_strcmp(lsh.name, rsh.name) < 0;
		}
	};

	template<auto orderNamespaceMeta>
	consteval auto CreateOrdersNameIDs() {
		constexpr auto orderMetaRange = meta::members_of(orderNamespaceMeta, meta::is_class);
		constexpr size_t orderCount = size(orderMetaRange);
		std::array<OrdersNameID, orderCount> nameIDs;

		// Gather names of all orders
		for (int orderIdx = 0; meta::info orderMeta : orderMetaRange) {
			nameIDs[orderIdx].name.str = meta::name_of(meta::type_of(orderMeta));
			nameIDs[orderIdx].name.len = const_strlen(nameIDs[orderIdx].name.str);
			nameIDs[orderIdx].ID = orderIdx;
			++orderIdx;
		}

		// Sort by name for rule binary search
		std::sort(nameIDs.begin(), nameIDs.end(), OrdersNameID::name_cmp);

		return nameIDs;
	}

	template<typename orderNsHelper>
	struct OrdersNameIDsHelper {
		static constexpr auto nameIDs 
			= CreateOrdersNameIDs<orderNsHelper::meta>();
	};

	template<size_t orderCount, typename orderNsHelper>
	struct OrdersDataImag {
		std::array<meta::info, orderCount> metas{};
		using nameIDsHelper = OrdersNameIDsHelper<orderNsHelper>;

		static constexpr int count = orderCount;
	};

	template<typename T1, typename T2>
	struct OrdersDataRI {
		T1 real;
		T2 imag;
	};

	template<auto namespaceMeta>
	consteval size_t CalcOrderRulesStorageSize() {
		size_t sum = 0;
		for (meta::info orderMeta : meta::members_of(namespaceMeta, meta::is_class)) {
			sum += size(meta::base_spec_range(orderMeta));
		}
		return sum;
	}

	template<typename orderNamespaceHelper>
	consteval auto CreateOrdersData() {
		// Order symbols are stored in struct/class names
		// Count all struct/class declarations in namespace
		constexpr auto orderNamespaceMeta = orderNamespaceHelper::meta;
		constexpr auto orderMetaRange = meta::members_of(orderNamespaceMeta, meta::is_class);
		constexpr size_t orderCount = size(orderMetaRange);

		// Order rules are based on inheritance
		// Count all struct/class declarations in namespace + all their bases(which define the rules)
		constexpr size_t orderRulesTotal = CalcOrderRulesStorageSize<orderNamespaceMeta>();
		constexpr size_t ordersStorageSize = orderCount + orderRulesTotal;

		OrdersDataReal<orderCount, ordersStorageSize> ordersReal{};
		OrdersDataImag<orderCount, orderNamespaceHelper> ordersImag{};
		using ordersImagType = decltype(ordersImag);

		// Gather metas of all orders
		for (int orderIdx = 0; meta::info orderMeta : orderMetaRange)
			ordersImag.metas[orderIdx++] = orderMeta;

		int rule_storage_pos = 0;
		for (int orderIdx = 0; meta::info orderMeta : orderMetaRange) {
			auto ruleMetaRange = meta::base_spec_range(orderMeta);
			auto ruleMetaRangeSize = size(ruleMetaRange);

			// Prepare rule storage
			ordersReal.rules       [orderIdx]         =	rule_storage_pos;
			ordersReal.rule_storage[rule_storage_pos] = ruleMetaRangeSize; // Per symbol, first elem is count of rules followed by rules
			rule_storage_pos += ruleMetaRangeSize + 1; // Advance by count of rules/bases + size for preparing the next iteration

			// Fill rule storage
			// Go through each each rule/base of a symbol
			int currRule = 0; // Index of rule/base for current order symbol
			for (auto rule : ruleMetaRange) {
				const char* ruleNameStr = meta::name_of(meta::type_of(rule));
				OrdersNameID ruleName{ {ruleNameStr, const_strlen(ruleNameStr) }, -1};

				// Binary search rule by name in nameID
				auto iter = lower_bound(ordersImagType::nameIDsHelper::nameIDs.begin(),
										ordersImagType::nameIDsHelper::nameIDs.end(),
										ruleName,
										OrdersNameID::name_cmp);
				auto ruleIdx = iter->ID; // Namespace order

				ordersReal.rule_data(orderIdx, currRule) = ruleIdx;
				++currRule;
			}

			++orderIdx;
		}

		return OrdersDataRI<decltype(ordersReal), decltype(ordersImag)>{ ordersReal, ordersImag };
	}

	template<size_t orderCount>
	struct OrdersCmpSwapMats {
		std::array<std::array<bool, orderCount>, orderCount> swap{};
		std::array<std::array<int, orderCount>, orderCount> cmp{};// < 
	};

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

} // namespace Meta