#pragma once
#include "optimizer/SimulatedAnnealing_Implementation1.h"
#include "optimizer/BranchAndBound1.h"
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
#define META_PRECOMPUTE_OR_CREATE_ARGUMENTS(order_namespace, function_namespace, optimizer_op_adapter, optimizer_op_params)\
			META_PRECOMPUTE_FUNC_IDXS(order_namespace, function_namespace, optimizer_op_adapter, optimizer_op_params)
#define META_INCLUDE_IDXS_HEADER(PATH, ON, FN, OP, PAR) "misc/empty.h"
#else
#define META_PRECOMPUTE_OR_CREATE_ARGUMENTS(order_namespace, function_namespace, optimizer_op_adapter, optimizer_op_params) META_CREATE_ARGUMENTS(order_namespace, function_namespace)
#define META_INCLUDE_IDXS_HEADER(PATH, ON, FN, OP, PAR) META_STRINGIFY(PATH/META_PRECOMPUTE_HEADER(ON, FN, OP, PAR))
#endif // defined(META_PRECOMPUTE)


// Strings and identifiers
#if !defined(META_PRECOMPUTE_HEADER_DIRECTORY)
#define META_PRECOMPUTE_HEADER_DIRECTORY ""
#endif

#define META_PRECOMPUTE_HEADER(ON, FN, OP, PAR) PrecomputedFuncIndxs_##ON##_##FN_##OP_##PAR
#define META_PRECOMPUTE_HEADER_FILENAME(ON, FN, OP, PAR) META_PRECOMPUTE_HEADER_DIRECTORY META_STRINGIFY(META_PRECOMPUTE_HEADER(ON, FN, OP, PAR))
#define META_PRECOMPUTE_PREC_FUNC_IDX_ARRAY_VAR(ON, FN, OP, PAR) precomputed_fidxs_##ON_##FN_##OP_##PAR

#if defined(META_PRECOMPUTE)
#define META_DEFINITIONS_ON_FN(ON, FN)			\
META_DEFINE_NAMESPACE_HELPERS(ON, FN)			\
META_DEFINE_ADDITIONAL_FUNCTION_INFO(ON, FN)
// END #define META_DEFINITIONS_ON_FN(ON, FN)

#define META_DEFINITIONS_ON_FN_OP(ON, FN, OP)	\
META_DEFINE_OPTIM_TO_FILE_FUNC(ON, FN, OP)
// END #define META_DEFINITIONS_ON_FN_OP(ON, FN, OP)
#else
#define META_DEFINITIONS_ON_FN(ON, FN) META_DEFINE_NAMESPACE_HELPERS(ON, FN)
#define META_DEFINITIONS_ON_FN_OP(ON, FN, OP)
#endif // defined(META_PRECOMPUTE)


// Scans functions and runs simulated annealing over them to determine the best ordering. Writes indexes to header file.
#define META_PRECOMPUTE_FUNC_IDXS(ON, FN, OP, PAR)																					\
namespace Meta																														\
{																																	\
inline int MetaOptimCleanFile ## ON ## FN ## OP ## PAR = DeleteIdxHeaderFile(META_PRECOMPUTE_HEADER_FILENAME(ON, FN, OP, PAR));		\
inline int MetaOptimToFile ## ON ## FN ## OP ## PAR = META_OPTIM_TO_FILE_FUNC(ON, FN, OP)<identity{}>(true, std::optional{ PAR },	\
														META_PRECOMPUTE_HEADER_FILENAME(ON, FN, OP, PAR),							\
														META_STRINGIFY(META_PRECOMPUTE_PREC_FUNC_IDX_ARRAY_VAR(ON, FN, OP, PAR)));	\
}
// END #define META_PRECOMPUTE_FUNC_IDXS(ON, FN, OP, PAR)	

#define META_DEFINE_OPTIM_TO_FILE_FUNC(ON, FN, OP)																		\
namespace Meta																											\
{																														\
template<auto SAInputPreprocessFunctor, typename paramsType>															\
inline int MetaOptimToFileHeaderFunc ## ON ## FN ## OP (bool write, std::optional<paramsType> paramsIn,					\
													    const char* header_filename, const char* idx_arr_name)			\
{																														\
	consteval {																											\
		using ON_Helper = META_NAMESPACE_HELPER_TYPE(ON, ON, FN);														\
		using FN_Helper = META_NAMESPACE_HELPER_TYPE(FN, ON, FN);														\
		using AdditionalFunctionInfo = AdditionalFunctionInfo ## ON ## FN;												\
																														\
		constexpr auto ordersDataRI = Meta::CreateOrdersData<ON_Helper>();												\
		Meta::meta::compiler.print("PRECOMPUTE_FUNC_IDXS: Created orders data.");										\
		constexpr auto paramsDataI = Meta::CreateParamsData<FN_Helper, ordersDataRI.imag>();							\
		Meta::meta::compiler.print("PRECOMPUTE_FUNC_IDXS: Created params data.");										\
		constexpr auto funcsDataRI =																					\
				Meta::CreateFuncsData<FN_Helper, ordersDataRI.imag, paramsDataI, AdditionalFunctionInfo>();				\
		Meta::meta::compiler.print("PRECOMPUTE_FUNC_IDXS: Created functions data.");									\
																														\
		->fragment {																									\
			using OrdersDataRealType [[maybe_unused]] = decltype(%{ ordersDataRI.real });								\
			using FuncsDataRealType [[maybe_unused]] = decltype(%{ funcsDataRI.real });									\
			constexpr auto ordersDataReal = %{ ordersDataRI.real };														\
			constexpr auto funcsDataReal = %{ funcsDataRI.real };														\
			auto ordersCmpSwapMats = CreateOrdersCmpSwapMats(ordersDataReal);											\
			auto funcsCmpSwapMats = CreateFuncsCmpSwapMats<ON_Helper, FN_Helper>(ordersCmpSwapMats, funcsDataReal);		\
			auto params = [:%{^paramsIn}:];																				\
																														\
			std::tuple pre_input{ ordersDataReal, funcsDataReal, ordersCmpSwapMats, funcsCmpSwapMats, params };			\
			std::tuple input = SAInputPreprocessFunctor.operator()(pre_input);											\
																														\
			auto newFuncsDataReal = Meta::OP<OrdersDataRealType, FuncsDataRealType,										\
											 decltype(ordersCmpSwapMats), decltype(funcsCmpSwapMats)>(input);			\
																														\
			if([:%{^write}:])																							\
				WriteIdxsToFile(newFuncsDataReal, [:%{^header_filename}:], [:%{^idx_arr_name}:]);						\
		};																												\
	}																													\
	return 0;																											\
}																														\
}																														\
// END #define META_DEFINE_OPTIM_TO_FILE_FUNC(ON, FN, OP)

#define META_OPTIM_TO_FILE_FUNC(ON, FN, OP) Meta::MetaOptimToFileHeaderFunc ## ON ## FN ## OP

// Creates variables in the global namespace to be used as function arguments
#define META_CREATE_ARGUMENTS(ON, FN)													\
consteval {																				\
	using ON_Helper = META_NAMESPACE_HELPER_TYPE(ON, ON, FN);							\
	using FN_Helper = META_NAMESPACE_HELPER_TYPE(FN, ON, FN);							\
																						\
	constexpr auto ordersDataRI = Meta::CreateOrdersData<ON_Helper>();					\
	Meta::meta::compiler.print("CREATE_ARGUMENTS: Created orders data.");				\
	constexpr auto paramsDataI = Meta::CreateParamsData<FN_Helper, ordersDataRI.imag>();\
	Meta::meta::compiler.print("CREATE_ARGUMENTS: Created params data.");				\
	Meta::CreateArguments<paramsDataI>();												\
	Meta::meta::compiler.print("CREATE_ARGUMENTS: Created arguments.");					\
}
// END #define META_CREATE_ARGUMENTS(ON, FN)

#define META_CALL_FUNCTIONS_OPTIMIZED(ON, FN, OP, PAR)													\
consteval {																								\
	using ON_Helper = META_NAMESPACE_HELPER_TYPE(ON, ON, FN);											\
	using FN_Helper = META_NAMESPACE_HELPER_TYPE(FN, ON, FN);											\
																										\
	constexpr auto ordersDataRI = Meta::CreateOrdersData<ON_Helper>();									\
	Meta::meta::compiler.print("CALL_FUNCTIONS_OPTIMIZED: Created orders data.");						\
	constexpr auto paramsDataI = Meta::CreateParamsData<FN_Helper, ordersDataRI.imag>();				\
	Meta::meta::compiler.print("CALL_FUNCTIONS_OPTIMIZED: Created params data.");						\
	constexpr auto funcsDataRI = Meta::CreateFuncsData<FN_Helper, ordersDataRI.imag, paramsDataI>();	\
	Meta::meta::compiler.print("CALL_FUNCTIONS_OPTIMIZED: Created functions data.");					\
																										\
	Meta::CallFuncs<funcsDataRI.imag,																	\
					funcsDataRI.real.funcs,																\
					META_PRECOMPUTE_PREC_FUNC_IDX_ARRAY_VAR(ON, FN, OP, PAR)>();						\
																										\
	Meta::meta::compiler.print("CALL_FUNCTIONS_OPTIMIZED: "												\
		"Listing function calls in optimized order from header file : ");								\
	for (int i = 0; i < funcsDataRI.real.funcs.size(); ++i)												\
		Meta::meta::compiler.print(Meta::meta::name_of(													\
			funcsDataRI.imag.metas[META_PRECOMPUTE_PREC_FUNC_IDX_ARRAY_VAR(ON, FN, OP, PAR)[i]]			\
		));																								\
}
// END #define META_CALL_FUNCTIONS_OPTIMIZED(ON, FN, OP, PAR)

#define META_DEFINE_NAMESPACE_HELPERS(ON, FN)					\
namespace Meta {												\
	struct namespaceHelper ## ON ## ON ## FN {					\
		static constexpr Meta::meta::info meta = ^ON;			\
	};															\
	struct namespaceHelper ## FN ## ON ## FN {					\
		static constexpr Meta::meta::info meta = ^FN;			\
	};															\
}
// END #define META_DEFINE_NAMESPACE_HELPERS(ON, FN)

#define META_NAMESPACE_HELPER_TYPE(NS, ON, FN) Meta::namespaceHelper ## NS ## ON ## FN

#define META_DEFINE_ADDITIONAL_FUNCTION_INFO(ON, FN)														\
namespace Meta {																							\
	struct AdditionalFunctionInfo ## ON ## FN {																\
			static constexpr int funcCount = size(Meta::meta::members_of(^ FN, Meta::meta::is_function));	\
			using orderNamespaceHelper = META_NAMESPACE_HELPER_TYPE(ON, ON, FN);							\
			using funcNamespaceHelper = META_NAMESPACE_HELPER_TYPE(FN, ON, FN);								\
	};																										\
}																											\
// END #define META_DEFINE_ADDITIONAL_FUNCTION_INFO(ON, FN) 

#define META_ADDITIONAL_FUNCTION_INFO_TYPE(ON, FN) Meta::AdditionalFunctionInfo ## ON ## FN

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

	using AdditionalFunctionInfo = typename decltype(newFuncsDataReal.funcs)::value_type::AdditionalInfo;
	using functionGetIDType = functionGetID<AdditionalFunctionInfo>;

	f << "#pragma once\n" << "#include <array>\n\n";
	f << std::format("inline constexpr std::array<int, {0}> {1}{{ {2} }};\n", newFuncsDataReal.count, precFuncIdxArrName, ContainerFormatAdapter{ newFuncsDataReal.funcs, AdaptType<functionGetIDType>() });

	f.close();
}
#endif // defined(META_PRECOMPUTE)

#define META_STRINGIFY_IMPL(X) #X
#define META_STRINGIFY(X) META_STRINGIFY_IMPL(X)

#define META_CONCAT_IMPL(X, Y) X##Y
#define META_CONCAT(X, Y) META_CONCAT_IMPL(X, Y)

struct identity {
	constexpr auto operator()(const auto& X) const { return X; }
};

//consteval void Debug(auto ordersRealIn, auto ordersImagIn, auto paramsImagIn, auto funcsRealIn, auto funcsImagIn, auto ordersCmpSwapMatsIn, auto funcsCmpSwapMatsIn) {
//	->fragment { std::cout << "///////////////////////////////\n" << "/////////////DEBUG/////////////\n" << "///////////////////////////////\n"; };
//	constexpr auto& orderNameIDs = decltype(ordersImagIn)::nameIDsHelper::nameIDs;
//	Meta::Print(ordersRealIn);
//	Meta::Print(ordersImagIn);
//	Meta::Print(ordersCmpSwapMatsIn);
//	Meta::Print(orderNameIDs);
//
//	->fragment {
//		constexpr auto ordersReal = % { ordersRealIn };
//		constexpr auto ordersCmpSwapMats = % { ordersCmpSwapMatsIn };
//
//		auto orderNameIDsReal = decltype(% { ordersImagIn })::nameIDsHelper::nameIDs;
//		std::sort(orderNameIDsReal.begin(), orderNameIDsReal.end(), Meta::OrderNameID::id_cmp);
//
//		for (int i = 0; i < orderNameIDsReal.size(); ++i) {
//			std::cout << orderNameIDsReal[i].name.str << " : ";
//			int orderID = orderNameIDsReal[i].ID;
//
//			int storageSize = ordersReal.rule_size(orderID);
//			for (int j = 0; j < storageSize; ++j) {
//				int ruleID = ordersReal.rule_data(orderID, j);
//				std::cout << orderNameIDsReal[ruleID].name.str << ", ";
//			}
//			std::cout << "\n";
//		}
//		std::cout << "\n";
//
//		struct print_mat {
//			void operator()(auto& mat, auto& orderNameIDsReal) {
//				int i, j;
//				for (i = 0; i < mat.size(); ++i) {
//					std::cout << std::format("Row {}\n", i);
//					for (j = 0; j < mat[i].size(); ++j)
//					{
//						std::cout << std::format("[{0}][{1}]: {2}  {3}  {4}\n", i, j, orderNameIDsReal[i].name.str, mat[i][j] == 1 ? "smaller" : "bigger", orderNameIDsReal[j].name.str);
//					}
//					std::cout << "\n";
//				}
//				std::cout << "\n\n";
//			}
//		};
//
//		std::cout << "Orderds CMP mat\n";
//		auto ordersCmp = ordersCmpSwapMats.cmp;
//		print_mat{}(ordersCmp, orderNameIDsReal);
//
//
//		std::cout << "Orderds SWAP mat\n";
//		auto ordersSwap = ordersCmpSwapMats.swap;
//		print_mat{}(ordersSwap, orderNameIDsReal);
//	};
//
//	constexpr auto& paramNameIDs = decltype(paramsImagIn)::nameIDsHelper::nameIDs;
//	Meta::Print(paramsImagIn);
//	Meta::Print(paramNameIDs);
//
//	constexpr auto& funcNameIDs = decltype(funcsImagIn)::nameIDsHelper::nameIDs;
//	Meta::Print(funcsRealIn);
//	Meta::Print(funcsImagIn);
//	Meta::Print(funcsCmpSwapMatsIn);
//	Meta::Print(funcNameIDs);
//
//	->fragment {
//		constexpr auto funcsReal = % { funcsRealIn };
//
//		auto funcNameIDsReal = decltype(% { funcsImagIn })::nameIDsHelper::nameIDs;
//		std::sort(funcNameIDsReal.begin(), funcNameIDsReal.end(), Meta::FuncNameID::id_cmp);
//
//		auto orderNameIDsReal = decltype(% { ordersImagIn })::nameIDsHelper::nameIDs;
//		std::sort(orderNameIDsReal.begin(), orderNameIDsReal.end(), Meta::OrderNameID::id_cmp);
//
//		auto paramNameIDsReal = decltype(% { paramsImagIn })::nameIDsHelper::nameIDs;
//		std::sort(paramNameIDsReal.begin(), paramNameIDsReal.end(), Meta::ParamNameID::id_cmp);
//
//		for (int i = 0; i < funcNameIDsReal.size(); ++i) {
//			int funcID = funcsReal.funcs[i].ID;
//			std::cout << "ID:" << funcID << " " << funcNameIDsReal[funcID].name.str << "(";
//
//			int j = 0;
//			auto paramSpan = funcsReal.f_params(funcID);
//			for (j = 0; j < paramSpan.len; ++j) {
//				std::cout << paramNameIDsReal[paramSpan[j]].cleanName.str << ", ";
//			}
//
//			auto orderSpan = funcsReal.f_orders(funcID);
//			for (j = 0; j < orderSpan.len; ++j) {
//				std::cout << orderNameIDsReal[orderSpan[j]].name.str << ", ";
//			}
//
//			std::cout << ")\n";
//		}
//		std::cout << "\n";
//	};
//}

} // namespace Meta