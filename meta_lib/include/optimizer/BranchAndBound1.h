//#pragma once
//#include <array>
//#include <vector>
//#include <span>
//#include <queue>
//
//namespace Meta
//{
//	template<typename OrdersDataRealType, typename FuncsDataRealType, typename OrdersCmpSwapMatsType, typename FuncsCmpSwapMatsType>
//	auto BBFunctionOrderOP(const auto& input);
//	
//    namespace BBFunctionOrder {
//
//        template<typename OrdersDataRealType, typename FuncsDataRealType, typename OrdersCmpSwapMatsType, typename FuncsCmpSwapMatsType>
//        struct BBStruct {
//
//            struct bound {
//                float value;
//                float cost;
//                int sn;
//
//                bool operator>(const bound& rhs) const {
//                    const auto asp1 = (value - cost) / (N - sn);
//                    const auto asp2 = (rhs.value - rhs.cost) / (N - rhs.sn);
//                    const auto ac1 = cost / sn;
//                    const auto ac2 = rhs.cost / rhs.sn;
//
//                    return (ac1 + asp1) > (ac2 + asp2);
//                    //return (value / sn) > (rhs.value / rhs.sn);
//                }
//            };
//
//            bound calc_lb(float cost_in, std::span<const int> current_options) {
//                bound lb_cost{ cost_in, cost_in, (int)(N - current_options.size()) };
//
//                for (int i = 0; i < current_options.size(); ++i)
//                    lb_cost.value += min_costs[current_options[i]];
//
//                return lb_cost;
//            }
//
//            struct ctx {
//                bound lb;
//                std::vector<int> sol;
//                std::vector<int> options;
//
//                bool operator>(const ctx& rhs) const {
//                    return lb > rhs.lb;
//                };
//            };
//
//            template<typename T>
//            class ext_priority_queue : public std::priority_queue<T, std::vector<T>, std::greater<T>>
//            {
//            public:
//                void remove_if(auto&& pred) {
//                    auto count = std::erase_if(this->c, std::forward<decltype(pred)>(pred));
//                    if (count)
//                        std::make_heap(this->c.begin(), this->c.end(), this->comp);
//                }
//
//                void cull(float ratio) {
//                    const size_t nth_idx = this->c.size() * ratio;
//                    std::nth_element(this->c.begin(), this->c.begin() + nth_idx, this->c.end(), std::greater{});
//                    const auto nth = this->c[nth_idx];
//
//                    remove_if([&nth](ctx& ctx) {
//                        return ctx > nth;
//                    });
//                }
//            };
//
//            void BB(const ctx& prev_ctx) {
//                // Add new nodes
//                const auto prev = prev_ctx.sol[prev_ctx.sol.size() - 1];
//                const auto prev_cost = prev_ctx.lb.cost;
//
//                for (int i = 0; i < prev_ctx.options.size(); ++i) {
//                    const auto curr = prev_ctx.options[i];
//                    const auto cost = prev_cost + costm[prev][curr];
//
//                    if (cost < hb.value)
//                    {
//                        ctx ctx;
//                        const std::span next_options{ &fswap[curr][1], (size_t)fswap[curr][0] };
//                        ctx.options.resize(next_options.size());
//                        const auto end = std::set_intersection(next_options.begin(), next_options.end(),
//                            prev_ctx.options.begin(), prev_ctx.options.end(),
//                            ctx.options.begin());
//                        const auto ctxOptionsSize = end - ctx.options.begin();
//
//                        if (prev_ctx.sol.size() + 1 + ctxOptionsSize != N)
//                            continue;
//
//                        ctx.options.resize(ctxOptionsSize);
//                        const bound lb = calc_lb(cost, ctx.options);
//
//                        if (lb.value < hb.value) {
//                            ctx.options.shrink_to_fit();
//
//                            ctx.sol.resize(prev_ctx.sol.size() + 1);
//                            std::copy(prev_ctx.sol.begin(), prev_ctx.sol.end(), ctx.sol.begin());
//                            ctx.sol[prev_ctx.sol.size()] = curr;
//
//                            ctx.lb = lb;
//
//                            if (ctx.sol.size() == N) {
//                                if (ctx.lb.cost < hb.value) {
//                                    hb = bound{ ctx.lb.cost, ctx.lb.cost, N };
//                                    std::copy(ctx.sol.begin(), ctx.sol.end(), best_sol.begin());
//                                    cleanpq = true;
//                                }
//                            }
//                            else {
//                                pqueue.push(ctx);
//                            }
//                        }
//                    }
//                }
//            }
//
//            void BranchAndBound() {
//                init();
//                hb = bound{ FLT_MAX, FLT_MAX, N };// 1.79;
//
//                // Add first level nodes
//                for (int i = 0; i < N; ++i) {
//                    const std::span options{ &fswap[i][1], (size_t)fswap[i][0] };
//
//                    if (options.size() != N - 1)
//                        continue;
//
//                    const bound lb = calc_lb(0, options);
//
//                    if (lb.value < hb.value) {
//                        ctx ctx;
//
//                        ctx.sol.push_back(i);
//                        ctx.options.resize(options.size());
//                        std::copy(options.begin(), options.end(), ctx.options.begin());
//                        ctx.lb = lb;
//
//                        pqueue.push(ctx);
//                    }
//                }
//
//                while (!pqueue.empty()) {
//                    if (cleanpq) {
//                        pqueue.remove_if(
//                            [](ctx& ctx) {
//                                return ctx.lb.value >= hb.value;
//                            });
//                        cleanpq = false;
//                    }
//
//                    if (pqueue.size() > N * std::sqrtf(N))
//                        pqueue.cull(0.55f);
//
//                    if (pqueue.empty())
//                        break;
//
//                    const auto& ctxref = pqueue.top();
//
//                    if (ctxref.lb.value < hb.value) {
//                        const auto ctx = pqueue.top();
//                        pqueue.pop();
//                        BB(ctx);
//                    }
//                    else
//                        pqueue.pop();
//                }
//            }
//
//            FuncsDataType fdata;
//            FuncsCmpSwapMatsType fmats;
//            decltype(fdata.funcs)& funcs = fdata.funcs;
//
//            static constexpr int funcCount = FuncsDataType::count;
//            std::array<std::array<float, funcCount>, funcCount> costm;
//            std::array<float, funcCount> min_costs;
//
//            std::array<int, N> best_sol;
//            int sol_num;
//
//            bound hb = { FLT_MAX, FLT_MAX, N };
//
//            ext_priority_queue<ctx> pqueue;
//            bool cleanpq = false;
//
//            void Init(const auto& ord_funcs_data_and_mat_tuple_and_saparams) {
//                fdata = std::get<1>(ord_funcs_data_and_mat_tuple_and_saparams);
//                fmats = std::get<3>(ord_funcs_data_and_mat_tuple_and_saparams);
//                auto optionalParams = std::get<4>(ord_funcs_data_and_mat_tuple_and_params);
//
//                for (int i = 0; i < N; ++i)
//                    costm[i][i] = FLT_MAX;
//                min_costs.fill(FLT_MAX);
//
//                for (int i = 0; i < N; ++i) {
//                    for (int j = i + 1; j < N; ++j) {
//                        const float cost = dist(mt);
//                        costm[i][j] = cost;
//                        costm[j][i] = cost;
//
//                        if (cost < min_costs[i])
//                            min_costs[i] = cost;
//                        if (cost < min_costs[j])
//                            min_costs[j] = cost;
//                    }
//                }
//            }
//
//        };
//    } // namespace BBFunctionOrder
//
//	template<typename OrdersDataRealType, typename FuncsDataRealType, typename OrdersCmpSwapMatsType, typename FuncsCmpSwapMatsType>
//	auto BBFunctionOrderOP(const auto& input) {
//        BBFunctionOrder::BBStruct<FuncsDataRealType::count> bb;
//
//               bb.Init(input);
//        return bb.BranchAndBound();
//	}
//} // namespace Meta