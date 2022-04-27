#pragma once
#include <array>
#include <vector>
#include <string_view>
#include <experimental/meta>
#include <experimental/compiler>

namespace Meta
{
	namespace std = ::std;
	namespace meta = std::experimental::meta;

	template<int sz>
	constexpr std::array<char, sz> explode(const char* src) {
		std::array<char, sz> arr{};
		for (int i = 0; i < sz - 1; ++i)
			arr[i] = src[i];
		arr[sz - 1] = 0;
		return arr;
	}

	struct sv {
		const char* str;
		int len;
	};

	constexpr int const_strlen(const char* str) {
		const char* cur = str;
		while (*(cur++) != '\0');
		return cur - str - 1;
	}

	constexpr int const_strcmp(const char* s1, const char* s2)
	{
		while (*s1 && (*s1 == *s2))
		{
			++s1;
			++s2;
		}
		return *s1 - *s2;
	}

	constexpr int const_strcmp(sv sv1, sv sv2)
	{
		const int minlen = sv1.len < sv2.len ? sv1.len : sv2.len;
		for (int i = 0; i < minlen; ++i)
			if (*(sv1.str++) != *(sv2.str++))
				return *(--sv1.str) - *(--sv2.str);

		return sv1.len - sv2.len;
	}

	constexpr bool const_strcmp_less(const char* s1, const char* s2) {
		return const_strcmp(s1, s2) < 0;
	}

	constexpr const char* remove_prefix_str(const char* s, int& slen, const char* pref, const int pref_len) {
		if (pref_len > slen)
			return s;

		for (int i = 0; i < pref_len; ++i)
			if (s[i] != pref[i])
				return s;

		slen -= pref_len;
		return s + pref_len;
	}

	constexpr bool is_ptr_type(const char* type_name, const int slen) {
		for (int i = 0; i < slen; ++i)
			if (type_name[i] == '*')
				return true;
		return false;
	}

	constexpr bool is_ptr_type(const sv& type_name) {
		return is_ptr_type(type_name.str, type_name.len);
	}

	constexpr const char* remove_namespace(const char* s, int& len) {
		//looking for ::
		for (int i = 0; i < len - 1/*2nd :*/; ++i)
			if (s[i] == ':' && s[i + 1] == ':') {
				len -= i + 2;
				return s + i + 2;
			}
		return s;
	}

	constexpr sv find_identifier(const char* s, int len) {
		for (int i = 0; i < len; ++i)
			if (s[i] == ' ')
				return { s, i };

		return { s, len };
	}

	constexpr sv clean_name(const char* dirty, int len) {
		const char* clean = remove_prefix_str(dirty, len, "volatile ", 9);
		clean = remove_prefix_str(clean, len, "const ", 6);
		clean = remove_prefix_str(clean, len, "volatile ", 9);
		clean = remove_namespace(clean, len);

		return find_identifier(clean, len);
	}

	constexpr sv clean_name(const char* dirty) {
		const int len = const_strlen(dirty);
		return clean_name(dirty, len);
	}

	constexpr sv clean_name(sv dirty) {
		return clean_name(dirty.str, dirty.len);
	}

	consteval sv clean_name(meta::info type) {
		const char* dirty = meta::name_of(meta::type_of(type));
		const int len = const_strlen(dirty);

		return clean_name(dirty, len);
	}

	constexpr int compare_type_names(const char* dirty_n1, const char* dirty_n2, bool isptr_n1, bool isptr_n2) {
		const auto clean_sv1 = clean_name(dirty_n1, const_strlen(dirty_n1));
		const auto clean_sv2 = clean_name(dirty_n2, const_strlen(dirty_n2));

		const auto cmp = const_strcmp(clean_sv1, clean_sv2);

		if (cmp == 0)
			return isptr_n1 - isptr_n2;

		return cmp;
	}

	consteval int compare_type_names(meta::info type1, meta::info type2) {
		return compare_type_names(
			meta::name_of(meta::type_of(type1)),
			meta::name_of(meta::type_of(type2)),
			meta::has_pointer_type(type1),
			meta::has_pointer_type(type2));
	}


	constexpr auto binary_search_it(auto first, auto last, const auto& value, auto cmp)
	{
		first = std::lower_bound(first, last, value, cmp);
		if (!(first == last) && !(cmp(value, *first)))
			return first;
		return last;
	}

} // namespace Meta

namespace std {
	//std::experimental::meta::detail::iterator<std::experimental::meta::front_member_fn,
	//											std::experimental::meta::next_member_fn,
	//											std::experimental::meta::is_function_fn>
	template<typename T1, typename T2, typename T3>
	_NODISCARD consteval auto next(experimental::meta::detail::iterator<T1, T2, T3> First, long long Off)  // increment iterator
	{
		for (auto i = 0; i < Off; ++i)
			First++;

		return First;
	}
}