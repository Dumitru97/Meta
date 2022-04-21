#pragma once
#include "1_Orders.h"
#include "misc/print.h"

namespace Meta
{
	struct ParamsNameID {
		const char* name;
		int ID;

		friend constexpr bool operator<(const ParamsNameID& lhs, const char* rhs) {
			return compare_type_names(lhs.name, rhs) < 0;
		}

		friend constexpr bool operator<(const char* lhs, const ParamsNameID& rhs) {
			return compare_type_names(lhs, rhs.name) < 0;
		}

		static consteval bool id_cmp(const ParamsNameID& lhs, const ParamsNameID& rhs) {
			return lhs.ID < rhs.ID;
		}

		static consteval bool name_cmp(const ParamsNameID& lhs, const ParamsNameID& rhs) {
			return compare_type_names(lhs.name, rhs.name) < 0;
		}

		static consteval bool name_eq(const ParamsNameID& lhs, const ParamsNameID& rhs) {
			return compare_type_names(lhs.name, rhs.name) == 0;
		}
	};

	template<size_t paramCount>
	struct ParamsDataImag {
		std::array<meta::info, paramCount> metas{};
		std::array<const char*, paramCount> meta_s{};
		static constexpr size_t count = paramCount;
	};

	template<auto namespaceMeta>
	consteval size_t CalcTotalParamAndOrderCount() {
		constexpr auto funcMetaRange = meta::members_of(namespaceMeta, meta::is_function);

		size_t count = 0;
		for (meta::info funcMeta : funcMetaRange) {
			count += size(meta::param_range(funcMeta));
		}
		return count;
	}


	template<auto namespaceMeta, auto ordersDataImag, auto totalParamAndOrderCount>
	consteval auto GetUniqueParamsUntruncated() {
		constexpr auto funcMetaRange = meta::members_of(namespaceMeta, meta::is_function);
		std::array<meta::info, totalParamAndOrderCount> allParamStorage{};

		// Gather metas, names and IDs of all params and orders from function parameters
		std::array<ParamsNameID, totalParamAndOrderCount> nameIDs{};
		for (int paramIdx = 0; meta::info funcMeta : funcMetaRange) {
			for (meta::info paramMeta : meta::param_range(funcMeta)) {
				allParamStorage[paramIdx] = paramMeta;

				nameIDs[paramIdx].name = meta::name_of(meta::type_of(paramMeta));
				nameIDs[paramIdx].ID = paramIdx;
				++paramIdx;
			}
		}

		// Remove duplicates
		std::sort(nameIDs.begin(), nameIDs.end(), ParamsNameID::name_cmp);
		auto nameIDsEnd = std::unique(nameIDs.begin(), nameIDs.end(), ParamsNameID::name_eq);

		// Gather names of all orders
		std::array<const char*, ordersDataImag.count> orderNames{};
		for (int orderIdx = 0; meta::info orderMeta : ordersDataImag.metas)
			orderNames[orderIdx++] = meta::name_of(meta::type_of(orderMeta));
		std::sort(orderNames.begin(), orderNames.end(), const_strcmp_less);

		// Remove orders
		std::array<ParamsNameID, totalParamAndOrderCount> uniqueNameIDs{};
		auto uniqueNameIDsEnd = std::set_difference(nameIDs.begin(), nameIDsEnd,
													orderNames.begin(), orderNames.end(),
													uniqueNameIDs.begin());
		const auto actual_size = std::distance(uniqueNameIDs.begin(), uniqueNameIDsEnd);

		// Pick and return unique parameters
		std::array<meta::info, totalParamAndOrderCount> uniqueParamStorage{};
		for (int i = 0; i < actual_size; ++i) {
			const auto uniqueIdx = uniqueNameIDs[i].ID;
			uniqueParamStorage[i] = allParamStorage[uniqueIdx];
		}

		return std::pair{ uniqueParamStorage, actual_size };
	};

	template<auto namespaceMeta, auto ordersDataImag>
	consteval auto CreateParamsData() {
		constexpr size_t paramAndOrderCount = CalcTotalParamAndOrderCount<namespaceMeta>();
		constexpr auto storageAndSize = GetUniqueParamsUntruncated<namespaceMeta, ordersDataImag, paramAndOrderCount>();

		constexpr auto actual_size = storageAndSize.second;
		ParamsDataImag<actual_size> paramsDataImag{};

		// Copy unique params from extra size storage to actual size storage
		for (int i = 0; i < actual_size; ++i) {
			paramsDataImag.metas[i] = storageAndSize.first[i];
			paramsDataImag.meta_s[i] = meta::name_of(storageAndSize.first[i]);
		}

		return paramsDataImag;
	}

} // namespace Meta