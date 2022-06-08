#pragma once
#include <utility>
#include <experimental/compiler>

namespace Meta
{
	namespace std = ::std;
	namespace meta = std::experimental::meta;

	inline constexpr std::array<const char*, 128> ch2str = { "\x00", "\x01", "\x02", "\x03", "\x04", "\x05", "\x06", "\x07", "\x08", "\x09", "\x0a",
															 "\x0b", "\x0c", "\x0d", "\x0e", "\x0f", "\x10", "\x11", "\x12", "\x13", "\x14", "\x15",
															 "\x16", "\x17", "\x18", "\x19", "\x1a", "\x1b", "\x1c", "\x1d", "\x1e", "\x1f", "\x20",
															 "\x21", "\x22", "\x23", "\x24", "\x25", "\x26", "\x27", "\x28", "\x29", "\x2a", "\x2b",
															 "\x2c", "\x2d", "\x2e", "\x2f", "\x30", "\x31", "\x32", "\x33", "\x34", "\x35", "\x36",
															 "\x37", "\x38", "\x39", "\x3a", "\x3b", "\x3c", "\x3d", "\x3e", "\x3f", "\x40", "\x41",
															 "\x42", "\x43", "\x44", "\x45", "\x46", "\x47", "\x48", "\x49", "\x4a", "\x4b", "\x4c",
															 "\x4d", "\x4e", "\x4f", "\x50", "\x51", "\x52", "\x53", "\x54", "\x55", "\x56", "\x57",
															 "\x58", "\x59", "\x5a", "\x5b", "\x5c", "\x5d", "\x5e", "\x5f", "\x60", "\x61", "\x62",
															 "\x63", "\x64", "\x65", "\x66", "\x67", "\x68", "\x69", "\x6a", "\x6b", "\x6c", "\x6d",
															 "\x6e", "\x6f", "\x70", "\x71", "\x72", "\x73", "\x74", "\x75", "\x76", "\x77", "\x78",
															 "\x79", "\x7a", "\x7b", "\x7c", "\x7d", "\x7e", "\x7f" };
	
	template<auto arr, int max, int curr = 0>
	consteval const char* ArrToString() {
		if constexpr (curr != max) {
			constexpr const char* str = ch2str[arr[curr]];
			return __concatenate(str, ArrToString<arr, max, curr + 1>());
		}
		else {
			return "";
		}
	}

	template<auto paramMeta>
	consteval const char* TypeToVarName() {
		constexpr const char* dirty_var_name = meta::name_of(meta::type_of(paramMeta));

		// Get a string view containing a valid variable name from given type
		constexpr int dirty_len = const_strlen(dirty_var_name);
		constexpr auto clean_sv = clean_name(dirty_var_name, dirty_len);
		constexpr int clean_sz = clean_sv.len;

		// Convert string view to const char*
		constexpr auto char_arr = explode<clean_sz + 1>(clean_sv.str); // + null terminating char
		constexpr const char* clean_str = ArrToString<char_arr, clean_sz>(); // __concatenate each

		// Decorate with suffix _gp(global pointer) or _gv(global variable)
		constexpr bool isPtr = meta::is_pointer_type(meta::type_of(paramMeta));
		constexpr const char* var_name = isPtr ? __concatenate(clean_str, "_gp") : __concatenate(clean_str, "_gv");

		return var_name;
	}

	template<auto paramsDataImag>
	consteval void CreateArguments() {
		template for (constexpr auto paramMeta : paramsDataImag.metas) {
			const char* var_name = TypeToVarName<paramMeta>();
			meta::compiler.print(__concatenate("Type ", meta::name_of(meta::type_of(paramMeta)), " to var name ", var_name));

			-> namespace(::) fragment namespace {
				std::remove_cvref_t<typename[:%{meta::type_of(paramMeta)}:]> [#[:^%{var_name}:]#] = {};
			};
		}
	}

} // namespace Meta