#pragma once
#include "optimizer/SimulatedAnnealing_Implementation1.h"
#include "3_Functions.h"
#include "misc/print.h"

#if defined(META_PRECOMPUTE)
#include "misc/func_format.h"
#include "misc/print.h"

#include <array>
#include <iostream>
#include <algorithm>
#else
#include "4_Arguments.h"
#include "5_Calls.h"
#endif // defined(META_PRECOMPUTE)

#include <fstream>
#include <filesystem>
#include <experimental/meta>
#include <experimental/compiler>

namespace Meta
{
namespace std = ::std;
namespace meta = std::experimental::meta;


// Main switch
// Selects whether to precompute function indexes and write to header file or create function arguments.
// This macro should be called alone in a header file. Include the header in the main project and in the precompute project.
// In this way the precompute project won't compile unnecessary source files from the main project.
#if defined(META_PRECOMPUTE)
#define META_PRECOMPUTE_OR_CREATE_ARGUMENTS(order_namespace, function_namespace) META_PRECOMPUTE_FUNC_IDXS(order_namespace, function_namespace)
#define META_INCLUDE_IDXS_HEADER(PATH, ON, FN) "misc/empty.h"
#else
#define META_PRECOMPUTE_OR_CREATE_ARGUMENTS(order_namespace, function_namespace) META_CREATE_ARGUMENTS(order_namespace, function_namespace)
#define META_INCLUDE_IDXS_HEADER(PATH, ON, FN) META_STRINGIFY(PATH/META_PRECOMPUTE_HEADER(ON, FN))
#endif // defined(META_PRECOMPUTE)


// Strings and identifiers
#if !defined(META_PRECOMPUTE_HEADER_DIRECTORY)
#define META_PRECOMPUTE_HEADER_DIRECTORY ""
#endif

#define META_PRECOMPUTE_HEADER(ON, FN) PrecomputedFuncIndxs_##ON##_##FN
#define META_PRECOMPUTE_HEADER_FILENAME(ON, FN) META_PRECOMPUTE_HEADER_DIRECTORY META_STRINGIFY(META_PRECOMPUTE_HEADER(ON, FN))
#define META_PRECOMPUTE_PREC_FUNC_IDX_ARRAY_VAR(ON, FN) precomputed_fidxs_##ON_##FN

// Scans functions and runs simulated annealing over them to determine the best ordering. Writes indexes to header file.
#define META_PRECOMPUTE_FUNC_IDXS(ON, FN)																	\
																											\
META_DEFINE_NAMESPACE_HELPERS(ON, FN)																		\
																											\
inline int MetaOptimToFileHeaderFunc ## ON ## FN () {														\
	consteval {																								\
		using ON_Helper = META_NAMESPACE_HELPER_TYPE(ON);													\
		using FN_Helper = META_NAMESPACE_HELPER_TYPE(FN);													\
																											\
		constexpr auto ordersDataRI = Meta::CreateOrdersData<ON_Helper>();									\
		Meta::meta::compiler.print("Created orders data.");													\
		constexpr auto paramsDataI = Meta::CreateParamsData<FN_Helper, ordersDataRI.imag>();				\
		Meta::meta::compiler.print("Created params data.");													\
																											\
	}																										\
	return 0;																								\
}																											\
																											\
namespace Meta																								\
{																											\
inline int MetaOptimCleanFile ## ON ## FN = DeleteIdxHeaderFile(META_PRECOMPUTE_HEADER_FILENAME(ON, FN));	\
inline int MetaOptimToFile ## ON ## FN = MetaOptimToFileHeaderFunc ## ON ## FN ();							\
}
// END #define META_PRECOMPUTE_FUNC_IDXS(ON, FN)	

//Print(paramsDataI);													
//constexpr auto funcsDataRI = Meta::CreateFuncsData<^ FN>(paramsDataI, ordersDataRI);				
//Meta::meta::compiler.print("Created functions data.");												


// Creates variables in the global namespace to be used as function arguments
#define META_CREATE_ARGUMENTS(ON, FN)												\
consteval {																			\
	constexpr auto ordersDataRI = Meta::CreateOrdersData<^ ON>();			\
	constexpr auto paramsDataI = Meta::CreateParamsData<^ FN, ordersDataRI.imag>();	\
	Meta::CreateArguments<paramsDataI>();											\
}
// END #define META_CREATE_ARGUMENTS(ON, FN)

#define META_CALL_FUNCTIONS_OPTIMIZED(ON, FN)													\
consteval {																						\
	constexpr auto ordersDataRI = Meta::CreateOrdersData<^ ON>();								\
	constexpr auto paramsData = Meta::CreateParamsData<^ FN, ordersDataRI.imag>();				\
	constexpr auto funcsDataRI = Meta::CreateFuncsData<^ FN>(paramsData, ordersDataRI);			\
	Meta::meta::compiler.print("Created functions data.");										\
																								\
	Meta::CallFuncs<funcsDataRI.imag,															\
					funcsDataRI.real.funcs,														\
					META_PRECOMPUTE_PREC_FUNC_IDX_ARRAY_VAR(ON, FN)>();							\
																								\
	Meta::meta::compiler.print("Listing function calls in optimized order from header file:");	\
	for (int i = 0; i < funcsDataRI.real.funcs.size(); ++i)										\
		Meta::meta::compiler.print(Meta::meta::name_of(Meta::meta::type_of(						\
			funcsDataRI.imag.metas[META_PRECOMPUTE_PREC_FUNC_IDX_ARRAY_VAR(ON, FN)[i]]			\
		)));																					\
}
// END #define META_CALL_FUNCTIONS_OPTIMIZED(ON, FN)

#define META_DEFINE_NAMESPACE_HELPERS(ON, FN)			\
namespace Meta {										\
	struct namespaceHelper##ON {						\
		static constexpr Meta::meta::info meta = ^ON;	\
	};													\
	struct namespaceHelper##FN {						\
		static constexpr Meta::meta::info meta = ^FN;	\
	};													\
}
// END #define META_DEFINE_NAMESPACE_HELPERS(ON, FN)

#define META_NAMESPACE_HELPER_TYPE(NS) Meta::namespaceHelper##NS

#if defined(META_PRECOMPUTE)
inline int DeleteIdxHeaderFile(const char* headerFilename) {
	bool exists = std::filesystem::exists(headerFilename);
	if (exists)
		std::filesystem::remove(headerFilename);
	return 0;
}

inline void WriteIdxsToFile(const auto& newFuncsDataReal, const char* headerFilename, const char* precFuncIdxArrName) {
	std::string_view headerPath = headerFilename;
	auto slashPos = headerPath.rfind('/');
	if (slashPos != std::string_view::npos) {
		std::string_view dirs = headerPath.substr(0, slashPos);
		std::filesystem::create_directories(dirs);
	}

	bool headerExists = std::filesystem::exists(headerFilename);
	if (headerExists) {
		std::cout << "Creating the same precomputed function indexes header file twice. "
					 "Use in only one translation unit.\n";
		std::abort();
	}

	std::ofstream f(headerFilename);
	if (!f.is_open()) {
		std::cout << "Failed to create precomputed indexes function header file.\n";
		std::abort();
	}

	f << "#pragma once\n" << "#include <array>\n\n";
	f << std::format("inline constexpr std::array<int, {0}> {1}{{ {2} }};\n", newFuncsDataReal.count, precFuncIdxArrName, ContainerFormatAdapter{ newFuncsDataReal.funcs, AdaptType<functionGetID>() });

	f.close();
}
#endif // defined(META_PRECOMPUTE)

#define META_STRINGIFY_IMPL(X) #X
#define META_STRINGIFY(X) META_STRINGIFY_IMPL(X)

#define META_CONCAT_IMPL(X, Y) X##Y
#define META_CONCAT(X, Y) META_CONCAT_IMPL(X, Y)


} // namespace Meta