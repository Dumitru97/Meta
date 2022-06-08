#pragma once
#include <tuple>

namespace Meta
{
	namespace std = ::std;

	template <class F, class Tup1, class Tup2, std::size_t... I1, std::size_t... I2>
	constexpr void custom_apply_impl(F&& f, Tup1&& t1, Tup2&& t2, std::index_sequence<I1...>, std::index_sequence<I2...>)
	{
		std::invoke(std::forward<F>(f), *std::get<I1>(std::forward<Tup1>(t1))..., std::get<I2>(std::forward<Tup2>(t2))...);
	}
	
	template <class F, class Tup1, class Tup2>
	constexpr void custom_apply(F&& f, Tup1&& t1, Tup2&& t2)
	{
		custom_apply_impl(
			std::forward<F>(f), std::forward<Tup1>(t1), std::forward<Tup2>(t2),
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tup1>>>{},
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tup2>>>{});
	}

	template<auto paramAddrMetaArrMeta, auto orderMetaArrMeta>
	consteval void PackParamsAndCall(auto funcMeta) {
		std::tuple paramTuple = std::make_tuple(...[:[:paramAddrMetaArrMeta:]:]...);
		std::tuple orderTuple = std::make_tuple(...[:[:orderMetaArrMeta:]:]...);

		->fragment {
			constexpr std::tuple paramTupleUnquoted = %{paramTuple};
			constexpr std::tuple orderTupleUnquoted = %{orderTuple};
			custom_apply([:%{funcMeta}:], paramTupleUnquoted, orderTupleUnquoted);
		};
	}

	template<auto paramRangeMeta, int size>
	consteval auto ParamRangeToVarAddrMetaArr() {
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
			meta_arr[i++] = ^(typename[:meta::type_of(order):]{});
		}
		return meta_arr;
	}

	template<auto funcsDataRI, auto funcsIdxArr>
	consteval void CallFuncs() {
		for_loop<0, funcsIdxArr.size()>([]<int funcIdx>() consteval {
			constexpr auto paramsSize = funcsDataRI.real.funcs[funcIdx].param_count();
			constexpr auto ordersSize = funcsDataRI.real.funcs[funcIdx].order_count();
			constexpr auto func       = funcsDataRI.imag.metas[funcIdx];
			constexpr auto fullRange  = meta::param_range(func);

			constexpr auto paramRange = meta::param_range(fullRange.begin(), std::next(fullRange.begin(), paramsSize));
			constexpr auto orderRange = meta::param_range(std::next(fullRange.begin(), paramsSize), fullRange.end());
			(void)paramRange;
			(void)orderRange;

			constexpr auto paramArr = ParamRangeToVarAddrMetaArr<^paramRange, paramsSize>();
			constexpr auto orderArr = OrderRangeToValueMetaArr<^orderRange, ordersSize>();
			(void)paramArr;
			(void)orderArr;

			PackParamsAndCall<^paramArr, ^orderArr>(func);
		});
	}

} // namespace Meta