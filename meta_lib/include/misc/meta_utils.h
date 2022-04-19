#pragma once
#include <array>
#include <string_view>
#include <experimental/meta>
#include <experimental/compiler>

namespace Meta
{
	namespace std = ::std;
	namespace meta = std::experimental::meta;

	struct sv {
		const char* str;
		int len;
	};

	struct svp {
		sv first;
		sv second;
	};

	//has terminating char
	template<int sz>
	constexpr std::array<char, sz> explode(const char* src) {
		std::array<char, sz> arr{};
		for (int i = 0; i < sz - 1; ++i)
			arr[i] = src[i];
		arr[sz - 1] = 0;
		return arr;
	}

	constexpr size_t const_strlen(const char* str) {
		const char* cur = str;
		while (*(cur++) != '\0');
		return cur - str - 1;
	}

	//returns last valid char idx + 1
	constexpr int find_valid_length(const char* src, int sz, const char* invalids, const int inv_len) {
		for (int i = sz - 1; i >= 0; --i) {
			int j = 0;
			for (; j < inv_len; j++)
				if (src[i] == invalids[j])
					break;
			if (j == inv_len)
				return i + 1;
		}
		return 0;
	}

	//returns last valid char idx + 1
	constexpr int find_valid_length(const char* src, int sz, const char invalid) {
		for (int i = sz - 1; i >= 0; --i) {
			if (src[i] == invalid)
				continue;
			else
				return i + 1;
		}
		return 0;
	}

	constexpr int const_strcmp(const char* s1, const char* s2)
	{
		while (*s1 && (*s1 == *s2))
		{
			s1++;
			s2++;
		}
		return *s1 - *s2;
	}

	constexpr bool const_streq(const char* s1, const char* s2) {
		return const_strcmp(s1, s2) == 0;
	}

	constexpr bool const_streq(const char* s1, const char* s2, const int l1, const int l2) {
		if (l1 != l2) return false;

		for (int i = 0; i < l1; ++i)
			if (s1[i] != s2[i]) return false;

		return true;
	}

	constexpr bool const_streq(const sv& sv1, const sv& sv2) {
		if (sv1.len != sv2.len) return false;

		for (int i = 0; i < sv1.len; ++i)
			if (sv1.str[i] != sv2.str[i]) return false;

		return true;
	}

	constexpr bool const_streq(const sv& sv1, const sv& sv2, int len) {
		for (int i = 0; i < len; ++i)
			if (sv1.str[i] != sv2.str[i]) return false;

		return true;
	}

	constexpr bool const_streq(const svp& svp1, const svp& svp2) {
		if ((svp1.first.len + svp1.second.len) != (svp2.first.len + svp2.second.len))
			return false;

		if (svp1.first.len != svp2.first.len) {
			//we change it later with substitutions
			sv b1, b2, b3, b4; //b1 is < len than b2
			if (svp1.first.len < svp2.first.len) {
				b1 = svp1.first;
				b2 = svp2.first;
				b3 = svp2.second;
				b4 = svp1.second;
			}
			else {
				b1 = svp2.first;
				b2 = svp1.first;
				b3 = svp1.second;
				b4 = svp2.second;
			}

			if (!const_streq(b1, b2, b1.len)) return false;

			b2.str += b1.len;
			b2.len -= b1.len;
			if (!const_streq(b2, b4, b2.len)) return false;

			b4.str += b2.len;
			b4.len -= b2.len;
			return const_streq(b3, b4, b3.len);
		}
		else { //first pair is equal len, means second is too
			return const_streq(svp1.first, svp2.first) && const_streq(svp1.second, svp2.second);
		}
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

	constexpr int remove_suffix_str_pos(const char* s, const int slen, const char* suffix, const int suffix_len) {
		const int str_pos = slen - suffix_len;
		if (str_pos < 0)
			return slen;

		for (int i = 0, k = str_pos; i < suffix_len; ++i, ++k)
			if (s[k] != suffix[i])
				return slen;
		return str_pos;
	}

	constexpr bool is_ptr_type(const char* type_name, const int slen) {
		for (int i = 0; i < slen; ++i)
			if (type_name[i] == '*')
				return true;
		return false;
	}

	constexpr svp remove_inside(const sv& s, const char* kw, const int kw_len) {
		if (s.len < kw_len)
			return { s, {"", 0} };
		//could check kw_len == 0

		const int stop = s.len - kw_len + 1;
		int i = 0;
		for (; i < stop; ++i)
			if (s.str[i] == kw[0]) {
				int k = 1;
				for (int j = i + 1; k < kw_len; ++j, ++k)
					if (s.str[j] != kw[k])
						break;
				if (k == kw_len)
					return { { s.str, i }, { s.str + i + kw_len, s.len - (i + kw_len)} };
			}

		return { s, {"", 0} };
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

	constexpr sv clean_name_cvref(const char* dirty, int len) {
		const char* clean = remove_prefix_str(dirty, len, "volatile ", 9);
		clean = remove_prefix_str(dirty, len, "const ", 6);
		clean = remove_namespace(clean, len);

		len = remove_suffix_str_pos(clean, len, "const", 5); //remove const pointer
		if (clean[len - 1] == ' ') len--; //remove white space

		int new_len = find_valid_length(clean, len, '&');
		if (new_len != len) //found &
			--new_len; //-1 for the space between type and &

		return sv{ clean, new_len };
	}

	constexpr sv clean_name(const char* dirty, int len) {
		const char* clean = remove_prefix_str(dirty, len, "volatile ", 9);
		clean = remove_prefix_str(dirty, len, "const ", 6);
		clean = remove_namespace(clean, len);

		len = remove_suffix_str_pos(clean, len, "const", 5); //remove const pointer
		if (clean[len - 1] == ' ') len--; //remove white space

		len = find_valid_length(clean, len, "& *", 3);

		return sv{ clean, len };
	}

	constexpr bool compare_type_names(const char* dirty_n1, const char* dirty_n2) {
		int len1 = const_strlen(dirty_n1);
		int len2 = const_strlen(dirty_n2);
		auto clean_sv1 = clean_name_cvref(dirty_n1, len1);
		auto clean_sv2 = clean_name_cvref(dirty_n2, len2);

		const auto svp1 = remove_inside(clean_sv1, "const", 5);
		const auto svp2 = remove_inside(clean_sv2, "const", 5);

		return const_streq(svp1, svp2);
	}

	consteval bool compare_type_names(meta::info reflect1, meta::info reflect2) {
		const char* dirty_n1 = meta::name_of(meta::type_of(reflect1));
		const char* dirty_n2 = meta::name_of(meta::type_of(reflect2));

		return compare_type_names(dirty_n1, dirty_n2);
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