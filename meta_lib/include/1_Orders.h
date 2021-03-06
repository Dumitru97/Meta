#pragma once

#include "misc/meta_utils.h"
#include <vector>
#include <algorithm>

namespace Meta
{
	//////////////////////////////////////////////
	// Data type used in non-consteval contexts //
	//////////////////////////////////////////////

	template<size_t orderCount, size_t rulesStorageSize>
	struct OrdersDataReal {
		using order_t = int;
		std::array<order_t, rulesStorageSize> rule_storage{}; // sz1 el11... sz2 el21...  ....
		std::array<int, orderCount> rules{}; // Holds indexes in rule_storage to desired order

		static constexpr int count = orderCount;

		constexpr auto  rule_size(int idx) const { return rule_storage[rules[idx]]; }
		constexpr auto& rule_size(int idx)       { return rule_storage[rules[idx]]; }
		constexpr auto  rule_data(int idx, int j) const { return rule_storage[rules[idx] + j + 1]; }
		constexpr auto& rule_data(int idx, int j)       { return rule_storage[rules[idx] + j + 1]; }
	};



	///////////////////////////////////////////
	// Data types used in consteval contexts //
	///////////////////////////////////////////

	struct OrderNameID {
		sv name;
		int ID;

		static constexpr bool name_cmp(const OrderNameID& lsh, const OrderNameID& rsh) {
			return const_strcmp(lsh.name, rsh.name) < 0; }
		static constexpr bool id_cmp(const OrderNameID& lsh, const OrderNameID& rsh) {
			return lsh.ID < rsh.ID; }
	};

	template<auto orderNamespaceMeta>
	consteval auto CreateOrderNameIDs() {
		constexpr auto orderMetaRange = meta::members_of(orderNamespaceMeta, meta::is_class);
		constexpr size_t orderCount = size(orderMetaRange);
		std::array<OrderNameID, orderCount> nameIDs;

		// Gather names of all orders
		for (int orderIdx = 0; meta::info orderMeta : orderMetaRange) {
			nameIDs[orderIdx].name.str = meta::name_of(meta::type_of(orderMeta));
			nameIDs[orderIdx].name.len = const_strlen(nameIDs[orderIdx].name.str);
			nameIDs[orderIdx].ID       = orderIdx;
			++orderIdx;
		}

		// Sort by name for rule binary search
		std::sort(nameIDs.begin(), nameIDs.end(), OrderNameID::name_cmp);

		return nameIDs;
	}

	// Structure passed through stages to provide const chars* as template parameters
	// Holds sorted names of orders
	template<typename orderNsHelper>
	struct OrderNameIDsHelper {
		static constexpr auto nameIDs
			= CreateOrderNameIDs<orderNsHelper::meta>();
	};

	template<size_t orderCount, typename orderNsHelper>
	struct OrdersDataImag {
		std::array<meta::info, orderCount> metas{};
		using nameIDsHelper = OrderNameIDsHelper<orderNsHelper>;

		static constexpr int count = orderCount;
	};



	/////////////////////////////////////////
	// Resulting output type of this stage //
	/////////////////////////////////////////

	template<typename T1, typename T2>
	struct OrdersDataRI {
		T1 real;
		T2 imag;
	};



	///////////////////////////////////
	// Output function of this stage //
	///////////////////////////////////

	template<auto namespaceMeta>
	consteval size_t CalcOrderRulesStorageSize() {
		size_t sum = 0;
		for (meta::info orderMeta : meta::members_of(namespaceMeta, meta::is_class)) {
			sum += size(meta::bases_of(orderMeta));
		}
		return sum;
	}

	template<typename orderNamespaceHelper>
	consteval auto CreateOrdersData() {
		// Order symbols are stored in struct/class names
		// Count all struct/class declarations in namespace
		constexpr auto   orderNsMeta    = orderNamespaceHelper::meta;
		constexpr auto   orderMetaRange = meta::members_of(orderNsMeta, meta::is_class);
		constexpr size_t orderCount     = size(orderMetaRange);

		// Order rules are based on inheritance
		// Count all struct/class declarations in namespace + all their bases(which define the rules)
		constexpr size_t orderRulesTotal   = CalcOrderRulesStorageSize<orderNsMeta>();
		constexpr size_t ordersStorageSize = orderCount + orderRulesTotal;

		OrdersDataReal<orderCount, ordersStorageSize> ordersReal{};
		OrdersDataImag<orderCount, orderNamespaceHelper> ordersImag{};
		using ordersImagType = decltype(ordersImag);

		// Imag part, gather metas of all orders
		for (int orderIdx = 0; meta::info orderMeta : orderMetaRange)
			ordersImag.metas[orderIdx++] = orderMeta;

		// Real part
		int rule_storage_pos = 0;
		for (int orderIdx = 0; meta::info orderMeta : orderMetaRange) {
			auto ruleMetaRange     = meta::bases_of(orderMeta);
			auto ruleMetaRangeSize = size(ruleMetaRange);

			// Prepare for rule storage
			ordersReal.rules       [orderIdx]		  = rule_storage_pos;
			ordersReal.rule_storage[rule_storage_pos] = ruleMetaRangeSize; // Per symbol, first elem is count of rules followed by the rules themselves
			rule_storage_pos += ruleMetaRangeSize + 1; // Advance by count + value of count for preparing the next iteration

			// Fill rule storage for current order
			// Go through each each rule/base of a symbol
			int currRule = 0; // Index of rule/base for current order symbol
			for (auto rule : ruleMetaRange) {
				const char* ruleNameStr = meta::name_of(meta::type_of(rule));
				OrderNameID ruleName{ {ruleNameStr, const_strlen(ruleNameStr) }, -1 };

				// Binary search rule by name in nameID
				auto iter = lower_bound(ordersImagType::nameIDsHelper::nameIDs.begin(),
										ordersImagType::nameIDsHelper::nameIDs.end(),
										ruleName,
										OrderNameID::name_cmp);
				auto ruleIdx = iter->ID; // One of the possible values for orderIdx

				ordersReal.rule_data(orderIdx, currRule) = ruleIdx;
				++currRule;
			}

			++orderIdx;
		}

		return OrdersDataRI<decltype(ordersReal), decltype(ordersImag)>{ ordersReal, ordersImag };
	}



	////////////////////////////////////////
	// Runtime preprocessing calculations //
	////////////////////////////////////////

	template<size_t orderCount>
	struct OrdersCmpSwapMats {
		std::array<std::array<bool, orderCount>, orderCount> swap{}; // <=>
		std::array<std::array<int , orderCount>, orderCount> cmp{};  // < 
	};

	// Returns e1 < e2, e2 < e1
	inline std::pair<bool, bool> OrdersCmp(const auto& expanded_bases, int primary, int secondary) {
		if (primary == secondary)
			return { true, true };

		auto& prim_exp_rules = expanded_bases[primary];
		auto& sec_exp_rules = expanded_bases[secondary];

		// Primary smaller than secondary
		for (int j = 0; j < sec_exp_rules.size(); ++j)
			if (primary == sec_exp_rules[j])
				return { true, false };

		// Secondary smaller than primary
		for (int i = 0; i < prim_exp_rules.size(); ++i)
			if (prim_exp_rules[i] == secondary)
				return { false, true };

		return { true, true };
	}

	// Add all bases recursively
	inline void AddBasesFrom(const auto& ordersReal, auto& exp_vec, int orderID) {
		// For each base/rule add their bases/rules
		for (int i = 0; i < ordersReal.rule_size(orderID); ++i) {
			const int base_id = ordersReal.rule_data(orderID, i);

			exp_vec.push_back(base_id); // Add current
			AddBasesFrom(ordersReal, exp_vec, base_id); // Add subrules
		}
	}

	inline void ComputeOrdersCmp(auto& ordersCmpSwapMats, const auto& ordersReal) {
		using order_t = typename std::remove_cvref_t<decltype(ordersReal)>::order_t;
		constexpr const int orderCount = ordersReal.count;

		using expanded_bases_t = std::array<std::vector<order_t>, orderCount>;
		expanded_bases_t expanded_bases;

		// Flatten rules in a vector for each order
		for (int orderID = 0; orderID < orderCount; ++orderID) {
			expanded_bases[orderID].push_back(orderID);
			AddBasesFrom(ordersReal, expanded_bases[orderID], orderID);
		}

		// Compute cmp and swap matrices
		for (int i = 0; i < orderCount; ++i) {
			for (int j = i; j < orderCount; ++j) {
				const auto& cmp_pair = OrdersCmp(expanded_bases, i, j);
				ordersCmpSwapMats.cmp[i][j] = cmp_pair.first;
				ordersCmpSwapMats.cmp[j][i] = cmp_pair.second;
				ordersCmpSwapMats.swap[i][j] = cmp_pair.first && cmp_pair.second;
				ordersCmpSwapMats.swap[j][i] = cmp_pair.first && cmp_pair.second;
			}
		}
	}

	inline auto CreateOrdersCmpSwapMats(const auto& ordersReal) {
		constexpr const int orderCount = ordersReal.count;

		OrdersCmpSwapMats<orderCount> ordersCmpSwapMats;
		ComputeOrdersCmp(ordersCmpSwapMats, ordersReal);

		return ordersCmpSwapMats;
	}

} // namespace Meta