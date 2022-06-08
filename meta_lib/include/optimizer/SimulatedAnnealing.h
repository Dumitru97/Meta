#pragma once
#include <random>
#include <iostream>
#include <format>

namespace Meta
{
	template<typename T>
	struct SimulatedAnnealingSettings {

		template<typename A = T>
		using temp_t = typename A::temp_t;

		template<typename A = T>
		using reps_t = typename A::reps_t;


		auto TransformInput(const auto& data) {
			return static_cast<T*>(this)->TransformInput(data);
		}

		auto InitialSolution(auto& data) {
			return static_cast<T*>(this)->InitialSolution(data);
		}

		auto CanBeOptimized(auto& data) {
			return static_cast<T*>(this)->CanBeOptimized(data);
		}

		auto Cost(const auto& solution) {
			return static_cast<T*>(this)->Cost(solution);
		}

		auto CostAndApply(auto& solution, const auto cost, const auto& delta) {
			return static_cast<T*>(this)->CostAndApply(solution, cost, delta);
		}

		auto NeighbourDelta(const auto& solution) {
			return static_cast<T*>(this)->NeighbourDelta(solution);
		}

		auto ApplyNeighbourDelta(auto& solution, const auto& delta) {
			return static_cast<T*>(this)->ApplyNeighbourDelta(solution, delta);
		}

		auto RevertNeighbourDelta(auto& solution, const auto& delta) {
			return static_cast<T*>(this)->RevertNeighbourDelta(solution, delta);
		}

		auto Probability(const auto negative_cost_diff, const auto temp) {
			return static_cast<T*>(this)->Probability(negative_cost_diff, temp);
		}

		auto ChangeSchedules(const auto k, auto& temp, auto& reps) {
			return static_cast<T*>(this)->ChangeSchedules(k, temp, reps);
		}

		auto InitSchedules(auto& temp, auto& reps) {
			return static_cast<T*>(this)->InitSchedules(temp, reps);
		}

		auto StopCheck(const auto& temp) {
			return static_cast<T*>(this)->StopCheck(temp);
		}

		auto TransformOutput(const auto& solution) {
			return static_cast<T*>(this)->TransformOutput(solution);
		}
	};

	template<class SASettingsSpec>
	auto SimulatedAnnealing(auto input_data) {
		SASettingsSpec settingsObj;
		SimulatedAnnealingSettings<SASettingsSpec>& set = settingsObj;
		std::uniform_real_distribution<float> udist(0.0f, 1.0f);
		std::random_device rd;

		auto data = set.TransformInput(input_data);
		auto sol = set.InitialSolution(data);

		if (!set.CanBeOptimized(sol))
			return set.TransformOutput(sol);

		typename SASettingsSpec::reps_t reps = 0;
		typename SASettingsSpec::temp_t temp = 0;
		set.InitSchedules(temp, reps);

		auto cost = set.Cost(sol);

		for (int k = 0;;) {
			int m = 0;
			while (m != reps) {
				const auto delta = set.NeighbourDelta(sol);
				const auto ncost = set.CostAndApply(sol, cost, delta);
				const auto diff = cost - ncost; // Current cost - new cost

				// New cost is higher and don't pick higher cost according to probability)
				if (diff < 0 && set.Probability(diff, temp) < udist(rd))
					set.RevertNeighbourDelta(sol, delta);
				else
					cost = ncost;

				++m;
			}
			++k;
			set.ChangeSchedules(k, temp, reps);

			if (set.StopCheck(temp))
				break;
		}

		auto output = set.TransformOutput(sol);
		return output;
	}

} // namespace Meta