#pragma once

#include "misc/meta_utils.h"
#include <vector>

namespace Meta
{
	template<size_t orderCount, size_t rulesStorageSize>
	struct OrdersDataReal {
		using order_t = int;

		std::array<std::array<bool, orderCount>, orderCount> swap_mat{};
		std::array<std::array<int, orderCount>, orderCount> cmp_mat{};// < 
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

	template<size_t orderCount>
	struct OrdersDataImag {
		std::array<meta::info, orderCount> metas{};
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

	//returns e1 < e2, e2 < e1
	constexpr std::pair<char, char> OrdersCmp(const auto& expanded_bases, int primary, int secondary) {
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

	constexpr bool And(const std::pair<char, char>& pair) {
		return pair.first && pair.second;
	}

	// Add all bases recursively
	constexpr void AddBasesFrom(const auto& orders, auto& vec, int idx) {
		// For each base/rule add their bases/rules
		for (int i = 0; i < orders.rule_size(idx); ++i) {
			const int base_idx = orders.rule_data(idx, i);

			vec.push_back(base_idx); // Add current
			AddBasesFrom(orders, vec, base_idx); // Add subrules
		}
	}

	constexpr void ComputeOrdersCmp(auto& orders) {
		using order_t = typename std::remove_cvref_t<decltype(orders)>::order_t;
		constexpr const int orderCount = orders.count;

		using expanded_bases_t = std::array<std::vector<order_t>, orderCount>; //O(N^2)
		expanded_bases_t expanded_bases;

		// Store rules in a flat vector instead of tree
		for (int i = 0; i < orderCount; ++i) {
			expanded_bases[i].push_back(i);
			AddBasesFrom(orders, expanded_bases[i], i);
		}

		// Compute cmp_mat and swap_mat
		for (int i = 0; i < orderCount; ++i) {
			for (int j = 0; j < i; ++j) {
				const auto& cmp_pair = OrdersCmp(expanded_bases, i, j);
				orders.swap_mat[i][j] = And(cmp_pair); // i < j && j < i aka canSwap?
				orders.cmp_mat[i][j] = cmp_pair.first; // if i < j
			}

			orders.swap_mat[i][i] = 1; // i < i
			orders.cmp_mat[i][i] = 3; // ==

			for (int j = i + 1; j < orderCount; ++j) {
				const auto& cmp_pair = OrdersCmp(expanded_bases, i, j);
				orders.swap_mat[i][j] = And(cmp_pair);
				orders.cmp_mat[i][j] = 1; //because i was declared before j
			}
		}
	}

	template<auto orderNamespaceMeta>
	consteval auto CreateOrdersData(bool computeSwap = true) {
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

		int idx = 0; // Index in rule_storage
		for (int i = 0; meta::info orderMeta : orderMetaRange) {
			ordersImag.metas[i] = orderMeta;
			ordersReal.rules[i] = idx; // Index in rule_storage
			ordersReal.rule_storage[idx] = size(meta::base_spec_range(orderMeta)); // Per symbol, first elem is count of rules followed by rules

			idx += size(meta::base_spec_range(orderMeta)) + 1; // Advance by num of rules/bases for preparing next iteration

			// Go through each each rule/base of a symbol
			int idx2 = 0; // Index of rule/base for current order symbol
			for (auto rule : meta::base_spec_range(orderMeta)) {
				// j is rule idx in orderMetaRange
				for (int j = 0; meta::info orderMetaCheck : orderMetaRange) {
					// Store the index of the rule's symbol
					// It will be found because rules/bases have to be declared beforehand
					if (meta::type_of(orderMetaCheck) == meta::type_of(rule)) {
						ordersReal.rule_data(i, idx2) = j;
						idx2++;
						break;
					}
					++j;
				}
			}

			++i;
		}

		if (computeSwap)
			ComputeOrdersCmp(ordersReal);

		return OrdersDataRI<decltype(ordersReal), decltype(ordersImag)>{ ordersReal, ordersImag };
	}

} // namespace Meta