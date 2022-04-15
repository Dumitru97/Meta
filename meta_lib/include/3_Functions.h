#pragma once
#include "2_Parameters.h"

namespace Meta
{
	template<typename T>
	struct span {
		constexpr int& operator[](int i) {
			return data[i];
		}

		constexpr int operator[](int i) const {
			return data[i];
		}

		int* data;
		int len;
	};

	struct function {
		int storageIdx; // paramIdxsStorage idx
		int paramEnd = -1;
		int orderEnd = -1;
		int ID = -1;

		constexpr int param_count() const {
			return paramEnd;
		}
		constexpr int order_count() const {
			return orderEnd - paramEnd;
		}
		constexpr int orders_end() const {
			return orderEnd;
		}
		constexpr int orders_start() const {
			return paramEnd;
		}
		constexpr int total_count() const {
			return orderEnd;
		}
	};

	using param_idx_t = int;
	template<size_t funcCount, size_t paramIdxsStorageSize_>
	struct FuncsDataReal {
		std::array<int, paramIdxsStorageSize_> paramIdxsStorage; //el11 ... el21 ...  ....
		std::array<function, funcCount> funcs{};

		inline static constexpr int count = funcCount;
		inline static constexpr int paramIdxsStorageSize = paramIdxsStorageSize_;

		constexpr auto& func(int i) {
			return funcs[i];
		}
		constexpr const auto& func(int i) const {
			return funcs[i];
		}
		constexpr span<int> f_all_params(int i) {
			return { &paramIdxsStorage[funcs[i].storageIdx], funcs[i].total_count() };
		}
		constexpr span<int> f_all_params(const function& f) {
			return { &paramIdxsStorage[f.storageIdx], f.total_count() };
		}
		constexpr span<int> f_params(int i) {
			return { &paramIdxsStorage[funcs[i].storageIdx], funcs[i].param_count() };
		}
		constexpr span<int> f_params(const function& f) {
			return { &paramIdxsStorage[f.storageIdx], f.param_count() };
		}
		constexpr span<int> f_orders(int i) {
			return { &paramIdxsStorage[funcs[i].storageIdx] + funcs[i].orders_start(), funcs[i].order_count() };
		}
		constexpr span<int> f_orders(const function& f) {
			return { &paramIdxsStorage[f.storageIdx] + f.orders_start(), f.order_count() };
		}
		constexpr span<const int> f_all_params(int i) const {
			return { &paramIdxsStorage[funcs[i].storageIdx], funcs[i].total_count() };
		}
		constexpr span<const int> f_all_params(const function& f) const {
			return { &paramIdxsStorage[f.storageIdx], f.total_count() };
		}
		constexpr span<const int> f_params(int i) const {
			return { &paramIdxsStorage[funcs[i].storageIdx], funcs[i].param_count() };
		}
		constexpr span<const int> f_params(const function& f) const {
			return { &paramIdxsStorage[f.storageIdx], f.param_count() };
		}
		constexpr span<const int> f_orders(int i) const {
			return { &paramIdxsStorage[funcs[i].storageIdx] + funcs[i].orders_start(), funcs[i].order_count() };
		}
		constexpr span<const int> f_orders(const function& f) const {
			return { &paramIdxsStorage[f.storageIdx] + f.orders_start(), f.order_count() };
		}
	};

	template<size_t funcCount>
	struct FuncsDataImag {
		std::array<meta::info, funcCount> metas{};
		inline static constexpr int count = funcCount;
	};

	template<typename T1, typename T2>
	struct FuncsDataRI {
		T1 real;
		T2 imag;
	};

	template<auto namespaceMeta>
	consteval auto CreateFuncsData(const auto& paramDataImag, const auto& orderDataImag) {
		constexpr auto funcMetaRange = meta::members_of(namespaceMeta, meta::is_function);
		constexpr size_t funcCount = size(funcMetaRange);

		constexpr size_t paramIdxsStorageSize = CalcTotalParamAndOrderCount<namespaceMeta>();

		FuncsDataReal<funcCount, paramIdxsStorageSize> funcsDataReal{};
		FuncsDataImag<funcCount> funcsDataImag{};

		int storeIdxOffset = 0;
		int i = 0;
		template for (constexpr meta::info funcMeta : funcMetaRange) {
			funcsDataImag.metas[i] = funcMeta;

			auto paramRange = meta::param_range(funcMeta);
			const auto totalParamRangeCount = size(paramRange);

			funcsDataReal.funcs[i].storageIdx = storeIdxOffset;
			funcsDataReal.funcs[i].paramEnd = -1;
			funcsDataReal.funcs[i].orderEnd = totalParamRangeCount;
			funcsDataReal.funcs[i].ID = i;

			int orderCount = 0;
			for (int j = 0; meta::info paramMeta : paramRange) {

				// Find param idx in paramData for current function parameter
				int paramIdx = 0;
				bool isParam = false;
				for (meta::info paramMetaCheck : paramDataImag.metas) {
					if (compare_type_names(paramMetaCheck, paramMeta)) {
						isParam = true;
						break;
					}
					++paramIdx;
				}

				if (isParam)
					funcsDataReal.paramIdxsStorage[storeIdxOffset + j++] = paramIdx; // Store idx
				else // isOrder
				{
					// Find order idx in orderData for current function parameter
					int orderIdx = 0;
					for (meta::info orderMetaCheck : orderDataImag.metas) {
						if (meta::type_of(orderMetaCheck) == meta::type_of(paramMeta))
							break;

						++orderIdx;
					}

					++orderCount;
					funcsDataReal.paramIdxsStorage[storeIdxOffset + j++] = orderIdx; // Store idx
				}
			}
			funcsDataReal.funcs[i].paramEnd = totalParamRangeCount - orderCount;
			storeIdxOffset += totalParamRangeCount;

			++i;
		}

		return FuncsDataRI<decltype(funcsDataReal), decltype(funcsDataImag)>{ funcsDataReal, funcsDataImag };
	}

} // namespace Meta