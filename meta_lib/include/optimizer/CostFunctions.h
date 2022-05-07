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

	inline float JaccardIndexOfSortedSets(const span<int>& v1, const span<int>& v2) {
		const int intersection_card = set_intersection_cardinal(v1.data, v1.data + v1.len, v2.data, v2.data + v2.len);
		return 1.0f - (float)intersection_card / (v1.len + v2.len - intersection_card);
	}

} // namespace Meta