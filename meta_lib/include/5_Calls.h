#pragma once
#include <tuple>

namespace Meta
{
	namespace std = ::std;

	template <class F, class Tuple1, std::size_t... I1, class Tuple2, std::size_t... I2>
	constexpr decltype(auto) custom_apply_impl(F&& f, Tuple1&& t1, std::index_sequence<I1...>, Tuple2&& t2, std::index_sequence<I2...>)
	{
		return std::invoke(std::forward<F>(f), *std::get<I1>(std::forward<Tuple1>(t1))..., std::get<I2>(std::forward<Tuple2>(t2))...);
	}
	
	template <class F, class Tuple1, class Tuple2>
	constexpr decltype(auto) custom_apply(F&& f, Tuple1&& t1, Tuple2&& t2)
	{
		return custom_apply_impl(
			std::forward<F>(f), std::forward<Tuple1>(t1),
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple1>>>{},
			std::forward<Tuple2>(t2),
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple2>>>{});
	}

	template<auto paramRange, auto orderRange>
	consteval void PackParamsAndCall(auto funcMeta) {
		std::tuple paramTuple = std::make_tuple(...[:[:paramRange:]:]...);
		std::tuple orderTuple = std::make_tuple(...[:[:orderRange:]:]...);


		//yo
		->fragment {
			constexpr std::tuple paramTupleUnquoted = %{paramTuple};
			constexpr std::tuple orderTupleUnquoted = %{orderTuple};
			custom_apply([:%{funcMeta}:], paramTupleUnquoted, orderTupleUnquoted);
		};
	}

	template<auto paramRangeMeta, int size>
	consteval auto ParamRangeToVarMetaArr() {
		std::array<meta::info, size> meta_arr{};

		int i = 0;
		template for (constexpr auto param : [:paramRangeMeta:] ) {
			constexpr const char* var_name = TypeToVarName<param>();
			(void)var_name;
			meta_arr[i++] = ^&[#[:^(var_name):]#];
		}
		return meta_arr;
	}

	template<auto orderRangeMeta, int size>
	consteval auto OrderRangeToValueMetaArr() {
		std::array<meta::info, size> meta_arr{};

		int i = 0;
		template for (constexpr auto order : [:orderRangeMeta:] ) {
			(void)order;
			meta_arr[i++] = ^ (typename[: meta::type_of(order) : ]{});
		}
		return meta_arr;
	}

	template<auto funcsDataImag, auto funcsArr, auto funcsIdxArr>
	consteval void CallFuncs() {
		template for (constexpr auto funcIdx : funcsIdxArr) {
			constexpr auto paramsSize = funcsArr[funcIdx].param_count();
			constexpr auto ordersSize = funcsArr[funcIdx].order_count();

			constexpr auto fullRange = meta::param_range(funcsDataImag.metas[funcIdx]);

			constexpr auto paramRange = meta::param_range(fullRange.begin(), std::next(fullRange.begin(), paramsSize));
			constexpr auto orderRange = meta::param_range(std::next(fullRange.begin(), paramsSize), fullRange.end());
			(void)paramRange;
			(void)orderRange;

			constexpr auto paramArr = ParamRangeToVarMetaArr<^ paramRange, paramsSize>();
			constexpr auto orderArr = OrderRangeToValueMetaArr<^ orderRange, ordersSize>();
			(void)paramArr;
			(void)orderArr;

			PackParamsAndCall<^ paramArr, ^ orderArr>(funcsDataImag.metas[funcIdx]);
		}
	}

} // namespace Meta