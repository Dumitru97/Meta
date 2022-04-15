#pragma once
#include "2_Parameters.h"

namespace Meta
{
	template<typename T>
	struct span {
		constexpr auto& operator[](int i) {
			return data[i];
		}

		constexpr auto operator[](int i) const {
			return data[i];
		}

		T* data;
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

		std::array<std::array<bool, funcCount>, funcCount> fswap_mat{};
		std::array<std::array<bool, funcCount>, funcCount> fcmp_mat{};  // <

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

	// Returns {f1 < f2, f2 < f1}
	constexpr std::pair<bool, bool> FuncOrdersCmp(const auto& funcsDataReal, const auto& ordersDataReal, const int fidx1, const int fidx2) {
		if (fidx1 == fidx2)
			return { true, true };

		auto fords1 = funcsDataReal.f_orders(fidx1);
		auto fords2 = funcsDataReal.f_orders(fidx2);

		// Different functions but same order symbols
		if (fords1.len == fords2.len) {
			bool same = true;
			for (int i = 0; i < fords1.len; ++i)
				if (fords1[i] != fords2[i]) {
					same = false;
					break;
				}
			if (same)
				return { true, true };
		}

		bool smaller1 = true;
		for (int i = 0; i < fords1.len; ++i)
			for (int j = 0; j < fords2.len; ++j)
				if (!ordersDataReal.cmp_mat[fords1[i]][fords2[j]]) {
					smaller1 = false;
					j = fords2.len;
					i = fords1.len;
				}

		bool smaller2 = true;
		for (int i = 0; i < fords2.len; ++i)
			for (int j = 0; j < fords1.len; ++j)
				if (!ordersDataReal.cmp_mat[fords2[i]][fords1[j]]) {
					smaller2 = false;
					j = fords1.len;
					i = fords2.len;
				}

		// Unlike orders, both functions can be wrongfully !smaller than eachother
		// so neither can be called before the other
		if (!smaller1 && !smaller2)
			throw;

		return { smaller1, smaller2 };
	}

	template<auto namespaceMeta>
	consteval auto CreateFuncsData(const auto& paramsDataImag, const auto& ordersDataRI) {
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
				for (meta::info paramMetaCheck : paramsDataImag.metas) {
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
					for (meta::info orderMetaCheck : ordersDataRI.imag.metas) {
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

		// Compute fcmp_mat and fswap_mat
		for (int i = 0; i < funcCount; ++i)
			for (int j = i; j < funcCount; ++j) {
				const std::pair<bool, bool> cmp = FuncOrdersCmp(funcsDataReal, ordersDataRI.real, i, j);
				funcsDataReal.fcmp_mat[i][j] = cmp.first;
				funcsDataReal.fcmp_mat[j][i] = cmp.second;
				funcsDataReal.fswap_mat[i][j] = cmp.first && cmp.second;
				funcsDataReal.fswap_mat[j][i] = cmp.first && cmp.second;
			}

		return FuncsDataRI<decltype(funcsDataReal), decltype(funcsDataImag)>{ funcsDataReal, funcsDataImag };
	}

} // namespace Meta