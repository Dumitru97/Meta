#pragma once
#include "1_Orders.h"
#include "misc/print.h"

namespace Meta
{
	///////////////////////////////////////////
	// Data types used in consteval contexts //
	///////////////////////////////////////////

	struct ParamNameID {
		sv cleanName;
		int ID_untruc;
		int ID;
		bool isPtr;

		static constexpr bool id_cmp(const ParamNameID& lhs, const ParamNameID& rhs) {
			return lhs.ID < rhs.ID;
		}
		static constexpr bool id_untruc_cmp(const ParamNameID& lhs, const ParamNameID& rhs) {
			return lhs.ID_untruc < rhs.ID_untruc;
		}
		static constexpr bool name_cmp(const ParamNameID& lhs, const ParamNameID& rhs) {
			auto res = const_strcmp(lhs.cleanName, rhs.cleanName);
			if (res == 0)
				return (lhs.isPtr - rhs.isPtr) < 0;

			return res < 0;
		}
		static constexpr bool name_eq(const ParamNameID& lhs, const ParamNameID& rhs) {
			return (const_strcmp(lhs.cleanName, rhs.cleanName) == 0) && (lhs.isPtr == rhs.isPtr);
		}
		friend constexpr bool operator<(const ParamNameID& lhs, sv rhs) {
			return const_strcmp(lhs.cleanName, rhs) < 0;
		}
		friend constexpr bool operator<(sv lhs, const ParamNameID& rhs) {
			return const_strcmp(lhs, rhs.cleanName) < 0;
		}
	};

	consteval size_t CalcTotalParamAndOrderCount(auto funcMetaRange) {
		size_t count = 0;
		for (meta::info funcMeta : funcMetaRange) {
			count += size(meta::parameters_of(funcMeta));
		}
		return count;
	}

	template<size_t totalParamAndOrderCount, typename ordersDataImagType>
	consteval auto CreateParamNameIDsUntrunc(auto funcMetaRange) {
		std::array<ParamNameID, totalParamAndOrderCount> nameIDs;

		// Gather metas, names and IDs of all params and orders from function parameters
		for (int paramIdx = 0; meta::info funcMeta : funcMetaRange) {
			for (meta::info paramMeta : meta::parameters_of(funcMeta)) {
				nameIDs[paramIdx].cleanName = clean_name(paramMeta);
				nameIDs[paramIdx].ID_untruc = paramIdx;
				nameIDs[paramIdx].ID        = paramIdx;
				nameIDs[paramIdx].isPtr     = meta::has_pointer_type(paramMeta);
				++paramIdx;
			}
		}

		// Remove duplicates
		std::sort(nameIDs.begin(), nameIDs.end(), ParamNameID::name_cmp);
		auto nameIDsEnd = std::unique(nameIDs.begin(), nameIDs.end(), ParamNameID::name_eq);

		// Clean order names
		std::array<sv, ordersDataImagType::count> orderNameIDs{};
		for (int orderIdx = 0; orderIdx < ordersDataImagType::count; ++orderIdx)
			orderNameIDs[orderIdx] = clean_name(ordersDataImagType::nameIDsHelper::nameIDs[orderIdx].name);

		// Remove orders
		std::array<ParamNameID, totalParamAndOrderCount> uniqueNameIDs{};
		auto uniqueNameIDsEnd = std::set_difference(nameIDs.begin(), nameIDsEnd,
													orderNameIDs.begin(), orderNameIDs.end(),
													uniqueNameIDs.begin());

		const auto actual_size = std::distance(uniqueNameIDs.begin(), uniqueNameIDsEnd);
		return std::pair{ uniqueNameIDs, actual_size };
	}

	template<typename funcNamespaceHelper, typename ordersDataImagType>
	consteval auto CreateParamNameIDs() {
		constexpr auto   funcNamespaceMeta       = funcNamespaceHelper::meta;
		constexpr auto   funcMetaRange           = meta::members_of(funcNamespaceMeta, meta::is_function);
		constexpr size_t totalParamAndOrderCount = CalcTotalParamAndOrderCount(funcMetaRange);

		constexpr auto result         = CreateParamNameIDsUntrunc<totalParamAndOrderCount, ordersDataImagType>(funcMetaRange);
		constexpr auto truncated_size = result.second;

		auto& uniqueNameIDsUntruncated = result.first;
		std::array<ParamNameID, truncated_size> nameIDs;

		// Copy unique nameIDs from extra size storage to actual size storage
		for (int i = 0; i < truncated_size; ++i) {
			nameIDs[i].cleanName = uniqueNameIDsUntruncated[i].cleanName;
			nameIDs[i].ID_untruc = uniqueNameIDsUntruncated[i].ID_untruc;
			nameIDs[i].ID        = i;
			nameIDs[i].isPtr     = uniqueNameIDsUntruncated[i].isPtr;
		}

		return nameIDs;
	}

	// Structure passed through stages to provide const chars* as template parameters
	// Holds sorted names of unique params, consequently the number of unique params
	template<typename funcNamespaceHelper, typename ordersDataImagType>
	struct ParamNameIDsHelper {
		static constexpr auto nameIDs
			= CreateParamNameIDs<funcNamespaceHelper, ordersDataImagType>();
	};




	/////////////////////////////////////////
	// Resulting output type of this stage //
	/////////////////////////////////////////

	template<size_t paramCount, typename funcNamespaceHelper, typename ordersDataImagType>
	struct ParamsDataImag {
		std::array<meta::info, paramCount> metas{};
		using nameIDsHelper = ParamNameIDsHelper<funcNamespaceHelper, ordersDataImagType>;

		static constexpr int count = paramCount;
	};



	///////////////////////////////////
	// Output function of this stage //
	///////////////////////////////////

	template<typename funcNamespaceHelper, auto ordersDataImag>
	consteval auto CreateParamsData() {
		constexpr auto   funcNamespaceMeta       = funcNamespaceHelper::meta;
		constexpr auto   funcMetaRange           = meta::members_of(funcNamespaceMeta, meta::is_function);
		constexpr size_t totalParamAndOrderCount = CalcTotalParamAndOrderCount(funcMetaRange);

		// Compute nameIDs
		using nameIDsHelper = ParamNameIDsHelper<funcNamespaceHelper, decltype(ordersDataImag)>;

		// Gather names of all function params and orders
		std::array<meta::info, totalParamAndOrderCount> allMetas{};
		for (int paramIdx = 0; meta::info funcMeta : funcMetaRange)
			for (meta::info paramMeta : meta::parameters_of(funcMeta))
				allMetas[paramIdx++] = paramMeta;

		constexpr auto truncated_size = nameIDsHelper::nameIDs.size();
		ParamsDataImag<truncated_size, funcNamespaceHelper, decltype(ordersDataImag)> paramsDataImag{};

		// Copy unique params from allMetas to output
		for (int i = 0; i < truncated_size; ++i)
			paramsDataImag.metas[i] = allMetas[nameIDsHelper::nameIDs[i].ID_untruc];

		return paramsDataImag;
	}

} // namespace Meta