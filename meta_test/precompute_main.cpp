#include "include/MetaArguments1.h"

//For default orders cost calculation
#include "include/simulation/DefaultCallOrderFuncNames.h"

template<typename FuncNameArrType>
struct DefaultOrderInputPreprocess {
	constexpr auto operator()(auto input) const {
		// Set SA params so that it isn't run but the cost is displayed
		auto& funcsData = std::get<1>(input);
		auto& optionalParams = std::get<4>(input);
		optionalParams = { .reps = 0, .temp = 0, .reps_increment = 0, .temp_decrement = 0, .pow_mult = 0 };

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
};

template<typename T>
DefaultOrderInputPreprocess(const T& cont)->DefaultOrderInputPreprocess<std::remove_cvref_t<T>>;

int main() {
	//Default order1 cost
	std::cout << "\n\n" << "DefaultOrder1" << "\n";
	constexpr auto defaultOrderInputPreprocess1 = DefaultOrderInputPreprocess{ defaultOrder1 };
	META_OPTIM_TO_FILE_FUNC(ON, FN, SAFunctionOrderOP)<defaultOrderInputPreprocess1>(false, std::optional{ sa_params }, nullptr, nullptr);

	//Default order2 cost
	std::cout << "\n\n" << "DefaultOrder2" << "\n";
	constexpr auto defaultOrderInputPreprocess2 = DefaultOrderInputPreprocess{ defaultOrder2 };
	META_OPTIM_TO_FILE_FUNC(ON, FN, SAFunctionOrderOP)<defaultOrderInputPreprocess2>(false, std::optional{ sa_params }, nullptr, nullptr);
}