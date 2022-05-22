#pragma once

namespace Meta
{
	template<typename SortedInputType1, typename SortedInputType2>
	inline int set_intersection_cardinal(SortedInputType1* data1, SortedInputType1* end1,
		SortedInputType2* data2, SortedInputType2* end2)
	{
		int count = 0;
		while (data1 != end1 && data2 != end2) {
			if (*data1 < *data2) {
				++data1;
			}
			else if (*data2 < *data1) {
				++data2;
			}
			else {
				++data1;
				++data2;
				++count;
			}
		}
		return count;
	}

	inline float JaccardIndexOfSortedSets(const auto& v1, const auto& v2) {
		const int intersection_card = set_intersection_cardinal(v1.begin(), v1.end(), v2.begin(), v2.end());
		return 1.0f - (float)intersection_card / (v1.size() + v2.size() - intersection_card);
	}

	template<typename T>
	inline float JaccardIndexOfSortedSets(span<T> v1, span<T> v2) {
		const int intersection_card = set_intersection_cardinal(v1.data, v1.data + v1.len, v2.data, v2.data + v2.len);
		return 1.0f - (float)intersection_card / (v1.len + v2.len - intersection_card);
	}

	template<size_t funcCount, typename orderNamespaceHelperIn, typename funcNamespaceHelperIn>
	struct FuncsJaccardCostMats {
		using cost_t = float;

		static inline std::array<std::array<cost_t, funcCount>, funcCount> cost;
		static inline std::array<cost_t, funcCount> min; // Other than self cost
		static inline std::array<cost_t, funcCount> avg;
		static inline bool isComputed = false;

		static void Init(const auto& fdata, const auto& funcs_perm) {
			if (isComputed)
				return;

			// Compute cost mats
			min.fill(FLT_MAX);

			for (int i = 0; i < funcCount; ++i) {
				for (int j = i; j < funcCount; ++j) {
					const auto func_i = funcs_perm[i];
					const auto func_j = funcs_perm[j];

					const float costval = JaccardIndexOfSortedSets(fdata.f_params(fdata.funcs[func_i]),
																   fdata.f_params(fdata.funcs[func_j]));
					cost[i][j] = costval;
					cost[j][i] = costval;

					if (i != j) {
						if (costval < min[i])
							min[i] = costval;
						if (costval < min[j])
							min[j] = costval;
					}
				}
			}

			for (int i = 0; i < funcCount; ++i) {
				cost_t sum = 0;
				for (int j = 0; j < funcCount; ++j) {
					if (i != j)
						sum += cost[i][j];
				}
				avg[i] = sum / (funcCount - 1);
			}

			isComputed = true;
		}
	};

} // namespace Meta