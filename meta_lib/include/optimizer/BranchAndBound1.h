#pragma once
#include <array>
#include <vector>
#include <span>
#include <queue>

namespace Meta
{
	template<typename OrdersDataRealType, typename FuncsDataRealType, typename OrdersCmpSwapMatsType, typename FuncsCmpSwapMatsType>
	auto BBFunctionOrderOP(const auto& input);
	
    namespace BBFunctionOrder {

        struct BBParams {
            size_t pqueueThresh;
            float cullRatio;
        };

        template<typename OrdersDataRealTypeIn, typename FuncsDataRealTypeIn, typename OrdersCmpSwapMatsTypeIn, typename FuncsCmpSwapMatsTypeIn>
        struct BBStruct {

            BBParams bb_params = { .pqueueThresh = std::max(funcCount * funcCount, 5000), .cullRatio = 0.55f };

            using FuncsDataType = std::remove_cvref_t<FuncsDataRealTypeIn>;
            using FuncsCmpSwapMatsType = std::remove_cvref_t<FuncsCmpSwapMatsTypeIn>;
            using OrderNamespaceHelper = typename FuncsDataType::AdditionalFunctionInfo::OrderNamespaceHelper;
            using FuncNamespaceHelper = typename FuncsDataType::AdditionalFunctionInfo::FuncNamespaceHelper;
            using OnlyMat = typename FuncsCmpSwapMatsType::OnlyMat;

            FuncsDataType fdata;
            FuncsCmpSwapMatsType fmats;
            decltype(fdata.funcs)& funcs = fdata.funcs;

            static constexpr int funcCount = FuncsDataType::count;
            static constexpr int N = funcCount;

            FuncsJaccardCostMats<funcCount, OrderNamespaceHelper, FuncNamespaceHelper> cmats;
            using cmatsType = decltype(cmats);
            using cost_t = typename decltype(cmats)::cost_t;

            FuncsDataType initFuncsData;
            float initcost;

            // Class definitions
        public:
            struct ctx {
                cost_t cost;
                cost_t lb;
                cost_t avs;

                std::vector<int> sol;
                std::vector<int> options;

                // Priority check
                bool operator<(const ctx& rhs) const {
                    int points = 0, rhs_points = 0;

                    const float constant = (rhs.cost - cost) + (rhs.avs - avs);
                    const float rhs_constant = -constant; //(cost - rhs.cost) + (avs - rhs.avs)

                    // Logic
                    //points += (cmatsType::avg[opt] < ((rhs.cost + rhs.avs) - (cost + (avs - cmatsType::avg[opt])) - cmatsType::min[opt]));
                    //rhs_points += (cmatsType::avg[opt] < ((rhs.cost + rhs.avs) - (cost + (avs - cmatsType::avg[opt])) - cmatsType::min[opt]));

                    for (int i = 0; i < options.size(); ++i) {
                        const int opt = options[i];
                        points += (0 < constant - cmatsType::min[opt]);
                    }

                    for (int i = 0; i < rhs.options.size(); ++i) {
                        const int opt = rhs.options[i];
                        rhs_points += (0 < rhs_constant - cmatsType::min[opt]);
                    }

                    return points < rhs_points;
                };
            };

            template<typename T>
            class ext_priority_queue : public std::priority_queue<T, std::vector<T>, std::less<T>>
            {
            public:
                void remove_if(auto&& pred) {
                    auto count = std::erase_if(this->c, std::forward<decltype(pred)>(pred));
                    if (count)
                        std::make_heap(this->c.begin(), this->c.end(), this->comp);
                }

                void cull(float ratio) {
                    const size_t nth_idx = this->c.size() * ratio;
                    std::nth_element(this->c.begin(), this->c.begin() + nth_idx, this->c.end(), std::less{});
                    const auto nth = this->c[nth_idx];

                    remove_if([&nth](ctx& ctx) {
                        return ctx < nth;
                    });
                }
            };


            // Primary member variables
            std::array<int, N> best_sol;
            bool found_sol = false;
            cost_t hb = FLT_MAX;

            ext_priority_queue<ctx> pqueue;
            bool cleanpq = false;


            // Computation methods
        public:
            cost_t calc_lb(cost_t cost_in, std::span<const int> current_options) {
                cost_t lb_cost = cost_in;

                for (int i = 0; i < current_options.size(); ++i)
                    lb_cost += cmats.min[current_options[i]];

                return lb_cost;
            }

            cost_t calc_avs(std::span<const int> current_options) {
                cost_t avs_cost = 0.0f;

                for (int i = 0; i < current_options.size(); ++i)
                    avs_cost += cmats.avg[current_options[i]];

                return avs_cost;
            }

            void BB(const ctx& prev_ctx) {
                // Add new nodes
                const auto prev = prev_ctx.sol[prev_ctx.sol.size() - 1];
                const auto prev_cost = prev_ctx.cost;

                for (int i = 0; i < prev_ctx.options.size(); ++i) {
                    const auto curr = prev_ctx.options[i];
                    const auto cost = prev_cost + cmats.cost[prev][curr];

                    if (cost < hb)
                    {
                        ctx ctx;
                        const std::span next_options{ fmats.cmp_only[curr].data(), (size_t)fmats.cmp_only[curr].count() };
                        ctx.options.resize(next_options.size());
                        const auto end = std::set_intersection(next_options.begin(), next_options.end(),
                                                               prev_ctx.options.begin(), prev_ctx.options.end(),
                                                               ctx.options.begin());
                        const auto ctxOptionsSize = end - ctx.options.begin();

                        if (prev_ctx.sol.size() + 1 + ctxOptionsSize != N)
                            continue;

                        ctx.options.resize(ctxOptionsSize);
                        const cost_t lb = calc_lb(cost, ctx.options);

                        if (lb < hb) {
                            ctx.options.shrink_to_fit();

                            ctx.sol.resize(prev_ctx.sol.size() + 1);
                            std::copy(prev_ctx.sol.begin(), prev_ctx.sol.end(), ctx.sol.begin());
                            ctx.sol[prev_ctx.sol.size()] = curr;

                            ctx.cost = cost;
                            ctx.lb = lb;
                            ctx.avs = calc_avs(ctx.options);

                            if (ctx.sol.size() == N) {
                                std::cout << "Found cost: " << ctx.cost << "\n";

                                if (ctx.cost < hb) {
                                    hb = ctx.cost;
                                    std::copy(ctx.sol.begin(), ctx.sol.end(), best_sol.begin());
                                    found_sol = true;
                                    cleanpq = true;
                                }
                            }
                            else {
                                pqueue.push(ctx);
                            }
                        }
                    }
                }
            }

            auto& BranchAndBound() {
                // Add first level nodes
                for (int i = 0; i < N; ++i) {
                    const std::span options{ fmats.cmp_only[i].data(), (size_t)fmats.cmp_only[i].count() };

                    if (options.size() != N - 1)
                        continue;

                    const cost_t lb = calc_lb(0, options);

                    if (lb < hb) {
                        ctx ctx;

                        ctx.sol.push_back(i);
                        ctx.options.resize(options.size());
                        std::copy(options.begin(), options.end(), ctx.options.begin());
                        ctx.cost = 0.0f;
                        ctx.lb = lb;
                        ctx.avs = calc_avs(ctx.options);

                        pqueue.push(ctx);
                    }
                }

                // Process the node queue
                while (!pqueue.empty()) {
                    if (cleanpq) {
                        pqueue.remove_if(
                            [this](ctx& ctx) {
                                return ctx.lb >= hb;
                            });
                        cleanpq = false;
                    }

                    if (pqueue.size() > bb_params.pqueueThresh)
                        pqueue.cull(bb_params.cullRatio);

                    if (pqueue.empty())
                        break;

                    const auto& ctxref = pqueue.top();

                    if (ctxref.lb < hb) {
                        const auto ctx = pqueue.top();
                        pqueue.pop();
                        BB(ctx);
                    }
                    else
                        pqueue.pop();
                }
                if (!found_sol) {
                    std::cout << "Did not find solution. Using returning input instead of output order.\n\n";
                    return initFuncsData;
                }

                for (int ordPos = 0; ordPos < N; ++ordPos) {
                    int ordFunc = -1;
                    for (int j = ordPos; j < N; ++j)
                        if (funcs[j].ID == best_sol[ordPos])
                            ordFunc = j;
                    std::swap(funcs[ordPos], funcs[ordFunc]);
                }

                // Cmp sanity check
                for (int i = 0; i < funcCount; ++i) {
                    bool isMin = true;
                    for (int j = i; j < funcCount; ++j) {
                        // Check if smaller than all
                        if (!fmats.cmp[funcs[i].ID][funcs[j].ID]) {
                            isMin = false;
                            break;
                        }
                    }
                    if (!isMin)
                        throw;
                }

                auto cost = Cost(funcs);
                auto costdiff = initcost - cost;
                std::cout << std::format("BranchAndBound - Valid input cost: {}, Output cost: {}, Diff: {}\n", initcost, cost, costdiff);

                if (costdiff < -1E-3f) {
                    std::cout << "Cost increase. Using returning input instead of output order.\n\n";
                    return initFuncsData;
                }
                std::cout << '\n';

                return fdata;
            }

            cost_t Cost(auto& funcs) {
                cost_t cost = 0.0f;
                for (int i = 0; i < funcCount - 1; ++i)
                    cost += cmats.cost[funcs[i].ID][funcs[i + 1].ID];

                return cost;
            }

            void Init(const auto& ord_funcs_data_and_mat_tuple_and_params) {
                fdata = std::get<1>(ord_funcs_data_and_mat_tuple_and_params);
                fmats = std::get<3>(ord_funcs_data_and_mat_tuple_and_params);
                auto optionalParams = std::get<4>(ord_funcs_data_and_mat_tuple_and_params);

                static_assert(funcCount == N);

                std::cout << "Running BranchAndBound.\n";
                initFuncsData = fdata;
                if (optionalParams)
                    bb_params = optionalParams.value();

                // Check if ordering is valid and initialize funcs_perm
                std::array<int, funcCount> funcs_perm{};
                auto isValidOrdering = ProduceInitialValidOrdering(funcs, funcs_perm, fmats, funcCount);
                if (!isValidOrdering)
                    std::cout << "Reordering functions into a valid ordering" << "\n";

                // Sanity check funcs_perm
                for (int i = 0; i < funcCount; ++i) {
                    int pos = funcs_perm[i];
                    if (funcs[pos].ID != i)
                        throw;
                }

                // Sort parameters for Jaccard index calculation
                for (int i = 0; i < funcCount; ++i) {
                    span<int> vec = fdata.f_params(i);
                    std::sort(vec.data, vec.data + vec.len);
                }

                // Init static Jaccard cost mats
                cmats.Init(fdata, funcs_perm);

                // Compute initial cost for difference
                initcost = Cost(funcs);
                hb = initcost;
            }

        };
    } // namespace BBFunctionOrder

	template<typename OrdersDataRealType, typename FuncsDataRealType, typename OrdersCmpSwapMatsType, typename FuncsCmpSwapMatsType>
	auto BBFunctionOrderOP(const auto& input) {
        BBFunctionOrder::BBStruct<OrdersDataRealType, FuncsDataRealType, OrdersCmpSwapMatsType, FuncsCmpSwapMatsType> bb;

               bb.Init(input);
        return bb.BranchAndBound();
	}
} // namespace Meta