#pragma once
#include "2_Parameters.h"

namespace Meta
{
	//////////////////////////////////////////////
	// Data types used in non-consteval contexts //
	//////////////////////////////////////////////

	template<typename AdditionalInfoIn>
	struct function {
		using AdditionalInfo = AdditionalInfoIn;

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
	template<size_t funcCount, size_t paramIdxsStorageSize_, typename AdditionalFunctionInfo>
	struct FuncsDataReal {
		using functionType = function<AdditionalFunctionInfo>;

		std::array<int, paramIdxsStorageSize_> paramIdxsStorage; //el11 ... el21 ...  ....
		std::array<functionType, funcCount> funcs{};

		static constexpr int count = funcCount;
		static constexpr int paramIdxsStorageSize = paramIdxsStorageSize_;

		constexpr auto& func(int i) {
			return funcs[i];
		}
		constexpr const auto& func(int i) const {
			return funcs[i];
		}
		constexpr span<int> f_all_params(int i) {
			return { &paramIdxsStorage[funcs[i].storageIdx], funcs[i].total_count() };
		}
		constexpr span<int> f_all_params(const functionType& f) {
			return { &paramIdxsStorage[f.storageIdx], f.total_count() };
		}
		constexpr span<int> f_params(int i) {
			return { &paramIdxsStorage[funcs[i].storageIdx], funcs[i].param_count() };
		}
		constexpr span<int> f_params(const functionType& f) {
			return { &paramIdxsStorage[f.storageIdx], f.param_count() };
		}
		constexpr span<int> f_orders(int i) {
			return { &paramIdxsStorage[funcs[i].storageIdx] + funcs[i].orders_start(), funcs[i].order_count() };
		}
		constexpr span<int> f_orders(const functionType& f) {
			return { &paramIdxsStorage[f.storageIdx] + f.orders_start(), f.order_count() };
		}
		constexpr span<const int> f_all_params(int i) const {
			return { &paramIdxsStorage[funcs[i].storageIdx], funcs[i].total_count() };
		}
		constexpr span<const int> f_all_params(const functionType& f) const {
			return { &paramIdxsStorage[f.storageIdx], f.total_count() };
		}
		constexpr span<const int> f_params(int i) const {
			return { &paramIdxsStorage[funcs[i].storageIdx], funcs[i].param_count() };
		}
		constexpr span<const int> f_params(const functionType& f) const {
			return { &paramIdxsStorage[f.storageIdx], f.param_count() };
		}
		constexpr span<const int> f_orders(int i) const {
			return { &paramIdxsStorage[funcs[i].storageIdx] + funcs[i].orders_start(), funcs[i].order_count() };
		}
		constexpr span<const int> f_orders(const functionType& f) const {
			return { &paramIdxsStorage[f.storageIdx] + f.orders_start(), f.order_count() };
		}
	};



	////////////////////////////////////
	// Data types used in any context //
	////////////////////////////////////

	struct FuncNameID {
		sv name;
		int ID;

		static constexpr bool name_cmp(const FuncNameID& lsh, const FuncNameID& rsh) {
			return const_strcmp(lsh.name, rsh.name) < 0;
		}

		static constexpr bool id_cmp(const FuncNameID& lsh, const FuncNameID& rsh) {
			return lsh.ID < rsh.ID;
		}
	};

	template<auto funcsNamespaceMeta>
	consteval auto CreateFuncNameIDs() {
		constexpr auto funcMetaRange = meta::members_of(funcsNamespaceMeta, meta::is_function);
		constexpr size_t funcCount = size(funcMetaRange);
		std::array<FuncNameID, funcCount> nameIDs;

		// Gather names of all funcs
		for (int funcIdx = 0; meta::info funcMeta : funcMetaRange) {
			nameIDs[funcIdx].name.str = meta::name_of(funcMeta);
			nameIDs[funcIdx].name.len = const_strlen(nameIDs[funcIdx].name.str);
			nameIDs[funcIdx].ID       = funcIdx;
			++funcIdx;
		}

		// Sort by name for rule binary search
		std::sort(nameIDs.begin(), nameIDs.end(), FuncNameID::name_cmp);

		return nameIDs;
	}

	// Holds sorted names of funcs
	template<typename funcsNsHelper>
	struct FuncNameIDsHelper {
		static constexpr auto nameIDs
			= CreateFuncNameIDs<funcsNsHelper::meta>();
	};



	///////////////////////////////////////////
	// Data types used in consteval contexts //
	///////////////////////////////////////////

	template<size_t funcCount, typename funcNamespaceHelper>
	struct FuncsDataImag {
		std::array<meta::info, funcCount> metas{};
		using nameIDsHelper = FuncNameIDsHelper<funcNamespaceHelper>;

		inline static constexpr int count = funcCount;
	};
	


	/////////////////////////////////////////
	// Resulting output type of this stage //
	/////////////////////////////////////////

	template<typename T1, typename T2>
	struct FuncsDataRI {
		T1 real;
		T2 imag;
	};



	///////////////////////////////////
	// Output function of this stage //
	///////////////////////////////////

	template<typename funcNamespaceHelper, auto ordersDataImag, auto paramsDataImag, typename AdditionalFunctionInfo = void>
	consteval auto CreateFuncsData() {
		constexpr auto funcMetaRange = meta::members_of(funcNamespaceHelper::meta, meta::is_function);

		constexpr size_t funcCount            = size(funcMetaRange);
		constexpr size_t paramIdxsStorageSize = CalcTotalParamAndOrderCount(funcMetaRange);

		FuncsDataReal<funcCount, paramIdxsStorageSize, AdditionalFunctionInfo> funcsDataReal{};
		FuncsDataImag<funcCount, funcNamespaceHelper> funcsDataImag{};

		// Gather data to be used in the order optimization step
		int storeIdxOffset = 0;
		int funcIdx = 0;
		for (meta::info funcMeta : funcMetaRange) {
			auto paramRange = meta::param_range(funcMeta);
			const auto funcParamRangeCount = size(paramRange);

			funcsDataImag.metas[funcIdx] = funcMeta;
			funcsDataReal.funcs[funcIdx].storageIdx = storeIdxOffset;
			funcsDataReal.funcs[funcIdx].paramEnd = -1;
			funcsDataReal.funcs[funcIdx].orderEnd = funcParamRangeCount;
			funcsDataReal.funcs[funcIdx].ID = funcIdx;

			// Fill paramIdxsStorage with idxs from ordersDataImag and paramsDataImag
			int orderCount = 0;
			for (int funcParamIdx = 0; meta::info paramMeta : paramRange)
			{
				// If order section hasn't started
				if (orderCount == 0) {
					// Find param idx in paramsData for current function parameter
					const auto& paramsNameIDs = decltype(paramsDataImag)::nameIDsHelper::nameIDs;
					ParamNameID value = { .cleanName = clean_name(paramMeta),
										   .ID = -1,
										   .isPtr = meta::has_pointer_type(paramMeta) };
					auto it = binary_search_it(paramsNameIDs.begin(), paramsNameIDs.end(), value, ParamNameID::name_cmp);

					// isParam?
					if (it != paramsNameIDs.end()) {
						funcsDataReal.paramIdxsStorage[storeIdxOffset + funcParamIdx++] = it->ID;
						continue;
					}
				}

				// Find order idx in ordersData for current function parameter
				const auto& ordersNameIDs = decltype(ordersDataImag)::nameIDsHelper::nameIDs;
				const char* orderName = meta::name_of(meta::type_of(paramMeta));
				OrderNameID value = { .name = { orderName, const_strlen(orderName) },
									   .ID = -1 };
				auto it = binary_search_it(ordersNameIDs.begin(), ordersNameIDs.end(), value, OrderNameID::name_cmp);
				if (it == ordersNameIDs.end())
					throw;

				funcsDataReal.paramIdxsStorage[storeIdxOffset + funcParamIdx++] = it->ID;
				++orderCount;
			}

			funcsDataReal.funcs[funcIdx].paramEnd = funcParamRangeCount - orderCount;

			storeIdxOffset += funcParamRangeCount;
			++funcIdx;
		}

		return FuncsDataRI<decltype(funcsDataReal), decltype(funcsDataImag)>{ funcsDataReal, funcsDataImag };
	}



	////////////////////////////////////////
	// Runtime preprocessing calculations //
	////////////////////////////////////////

	template<size_t funcCount, typename orderNamespaceHelperIn, typename funcNamespaceHelperIn>
	struct FuncsCmpSwapMats {
		static inline std::array<std::array<bool, funcCount>, funcCount> swap{};
		static inline std::array<std::array<bool, funcCount>, funcCount> cmp{};  // <
		static inline bool isComputed = false;

		using SwapMatType = decltype(swap);
		using CmpMatType = decltype(cmp);

		template<typename AdditionalFunctionInfo>
		friend bool operator<(const function<AdditionalFunctionInfo>& lhs,
							  const function<AdditionalFunctionInfo>& rhs);

		template<typename AdditionalFunctionInfo>
		friend bool operator<=>(const function<AdditionalFunctionInfo>& lhs,
								const function<AdditionalFunctionInfo>& rhs);
	};

	// Returns {f1 < f2, f2 < f1}
	inline std::pair<bool, bool> FuncOrdersCmp(const auto& ordersCmpSwapMats, const auto& funcsDataReal, const int fidx1, const int fidx2) {
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
				if (!ordersCmpSwapMats.cmp[fords1[i]][fords2[j]]) {
					smaller1 = false;
					j = fords2.len;
					i = fords1.len;
				}

		bool smaller2 = true;
		for (int i = 0; i < fords2.len; ++i)
			for (int j = 0; j < fords1.len; ++j)
				if (!ordersCmpSwapMats.cmp[fords2[i]][fords1[j]]) {
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

	template<typename orderNamespaceHelper, typename funcNamespaceHelper>
	inline auto CreateFuncsCmpSwapMats(const auto& ordersCmpSwapMats, const auto& funcsDataReal) {
		constexpr auto funcCount = funcsDataReal.count;
		FuncsCmpSwapMats<funcCount, orderNamespaceHelper, funcNamespaceHelper> funcsCmpSwapMats;

		if(funcsCmpSwapMats.isComputed)
			return funcsCmpSwapMats;

		// Compute cmp and swap matrices
		for (int i = 0; i < funcCount; ++i)
			for (int j = i; j < funcCount; ++j) {
				const std::pair<bool, bool> cmp = FuncOrdersCmp(ordersCmpSwapMats, funcsDataReal, i, j);
				funcsCmpSwapMats.cmp[i][j] = cmp.first;
				funcsCmpSwapMats.cmp[j][i] = cmp.second;
				funcsCmpSwapMats.swap[i][j] = cmp.first && cmp.second;
				funcsCmpSwapMats.swap[j][i] = cmp.first && cmp.second;
			}

		funcsCmpSwapMats.isComputed = true;
		return funcsCmpSwapMats;
	}

} // namespace Meta