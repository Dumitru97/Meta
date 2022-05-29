#include "include/MetaArguments1.h"
#include "include/MetaArguments2.h"

//For default orders cost calculation
#include "include/simulation/DefaultCallOrderFuncNames.h"

template<typename FuncNameArrType, typename ParamType>
struct DefaultOrderInputPreprocess {
	auto operator()(auto input) const {
		// Set SA params so that it isn't run but the cost is displayed
		auto& funcsData = std::get<1>(input);
		auto& optionalParams = std::get<4>(input);
		optionalParams = custom_params;

		using FN_Helper = META_NAMESPACE_HELPER_TYPE(FN, ON, FN);
		const auto& funcNameIDs = Meta::FuncNameIDsHelper<FN_Helper>::nameIDs;

		// Preprocess defaultOrderFuncNameArr into a nameID vector
		const auto& defNameArr = defaultOrderFuncNameArr;
		std::vector<Meta::FuncNameID> defaultOrderFuncNameIDs(defNameArr.size());
		for (int idx = 0; auto& defNameID : defaultOrderFuncNameIDs) {
			defNameID = Meta::FuncNameID{ .name = { .str = defNameArr[idx],
													.len = (int)Meta::const_strlen(defNameArr[idx]) },
										  .ID = idx };
			++idx;
		}

		// Sort default order funcs by name, same as Meta::FuncNameIDsHelper<FN_Helper>::nameIDs
		std::sort(defaultOrderFuncNameIDs.begin(), defaultOrderFuncNameIDs.end(), Meta::FuncNameID::name_cmp);

		// Now that names both containers are in the same order, we order funcsData.funcs in the default order into a buffer
		using functionType = typename decltype(funcsData.funcs)::value_type;
		std::vector<functionType> functionDataBuffer(funcsData.count);
		for (int nameIDIdx = 0; nameIDIdx < defNameArr.size(); ++nameIDIdx) {
			const auto bufferIdx = defaultOrderFuncNameIDs[nameIDIdx].ID;
			const auto funcDataIdx = funcNameIDs[nameIDIdx].ID;

			functionDataBuffer[bufferIdx] = funcsData.funcs[funcDataIdx];
		}

		// Copy the buffer into funcsData
		std::copy(functionDataBuffer.begin(), functionDataBuffer.end(), funcsData.funcs.begin());

		return input;
	}

	const FuncNameArrType& defaultOrderFuncNameArr;
	ParamType custom_params;
};

template<typename FuncNameArrType, typename ParamType>
DefaultOrderInputPreprocess(const FuncNameArrType& cont, ParamType custom_params)
	->DefaultOrderInputPreprocess<std::remove_cvref_t<FuncNameArrType>, ParamType>;

int main() {
	constexpr Meta::SAFunctionOrder::SAParams empty_sa_params = {.reps = 0, .temp = 0, .reps_increment = 0, .temp_decrement = 0, .pow_mult = 0 };

	//Default order1 cost
	std::cout << "DefaultOrder1" << "\n";
	auto defaultSAOrderInputPreprocess1 = DefaultOrderInputPreprocess{ defaultOrder1, empty_sa_params };
	META_OPTIM_TO_FILE_FUNC(ON, FN, SAFunctionOrderOP)(false, defaultSAOrderInputPreprocess1, std::optional{ sa_params }, nullptr, nullptr);

	//Default order2 cost
	std::cout << "DefaultOrder2" << "\n";
	auto defaultSAOrderInputPreprocess2 = DefaultOrderInputPreprocess{ defaultOrder2, empty_sa_params };
	META_OPTIM_TO_FILE_FUNC(ON, FN, SAFunctionOrderOP)(false, defaultSAOrderInputPreprocess2, std::optional{ sa_params }, nullptr, nullptr);

	//SA from default order2
	std::cout << "SA from DefaultOrder2" << "\n";
	auto defaultSAOrderInputPreprocess3 = DefaultOrderInputPreprocess{ defaultOrder2, sa_params };
	META_OPTIM_TO_FILE_FUNC(ON, FN, SAFunctionOrderOP)(false, defaultSAOrderInputPreprocess3, std::optional{ sa_params }, nullptr, nullptr);

	//BB from default order2
	std::cout << "BB from DefaultOrder2" << "\n";
	auto defaultBBOrderInputPreprocess3 = DefaultOrderInputPreprocess{ defaultOrder2, bb_params };
	META_OPTIM_TO_FILE_FUNC(ON, FN, BBFunctionOrderOP)(false, defaultBBOrderInputPreprocess3, std::optional{ bb_params }, nullptr, nullptr);
}