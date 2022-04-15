#pragma once
#include "1_Orders.h"

namespace Meta
{
	template<size_t paramCount>
	struct ParamsDataImag {
		std::array<meta::info, paramCount> metas{};
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

		std::array<meta::info, totalParamAndOrderCount> maxParamStorage{};
		size_t idx = 0;
		for (meta::info funcMeta : funcMetaRange) {
			for (meta::info paramMeta : meta::param_range(funcMeta)) {
				bool pass = true;

				// Check if this parameter meta has already been added to storage
				for (int i = 0; i < idx; ++i)
					if (compare_type_names(maxParamStorage[i], paramMeta))
						pass = false;

				// Check if this parameter is an order symbol
				for (int i = 0; i < ordersDataImag.count; ++i)
					if (meta::type_of(ordersDataImag.metas[i]) == meta::type_of(paramMeta))
						pass = false;

				if (pass)
					maxParamStorage[idx++] = paramMeta;
			}
		}
		const auto actual_size = idx;
		return std::pair{ maxParamStorage, actual_size };
	};

	template<auto namespaceMeta, auto ordersDataImag>
	consteval auto CreateParamsData() {
		constexpr size_t paramAndOrderCount = CalcTotalParamAndOrderCount<namespaceMeta>();

		constexpr auto storageAndSize = GetUniqueParamsUntruncated<namespaceMeta, ordersDataImag, paramAndOrderCount>();
		constexpr auto actual_size = storageAndSize.second;
		ParamsDataImag<actual_size> paramsDataImag{};

		// Copy unique params from extra size storage to actual size storage
		for (int i = 0; i < actual_size; ++i)
			paramsDataImag.metas[i] = storageAndSize.first[i];

		return paramsDataImag;
	}

} // namespace Meta