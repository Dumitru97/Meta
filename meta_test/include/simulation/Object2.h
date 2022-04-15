#pragma once
using Objects2 = Objects;

using namespace ON;

INLINE void Objs2MovedByActs1(Objects2& objs2, const Actors1& acts1, OObject2MovedBy, OActor1UpdateDone) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	//constexpr float dist_tresh = 1.744179f;

	for (const auto& act : acts1)
		for (int i = 0; i < objs2.size(); ++i) {
			//const auto& obj = objs2[i].obj;
			//auto& obj_next = objs2[i].obj_next;

			//if (distsq(act.pos, obj.pos) > dist_tresh * dist_tresh) {
				objs2[i].obj_next.pos.y += 0.27056614f;
			//}
		}
}

INLINE void Objs2WorldWrap(Objects2& objs2, OObject2Wrap, OObjectAnyWrap) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	for (auto& obj_wrapper : objs2) {
		auto& objn = obj_wrapper.obj_next;

		objn.pos.x = objn.pos.x.apply(fmodf_width);
		objn.pos.x(Vc::isnegative(objn.pos.x)) += world_width;

		objn.pos.y = objn.pos.y.apply(fmodf_height);
		objn.pos.y(Vc::isnegative(objn.pos.y)) += world_height;
	}
}

INLINE void Objs2Draw(const Objects2& objs2, OObjectAnyWrapDone) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	constexpr int paint_val = 11;

	for (const auto& obj_wrapper : objs2) {
		const auto& objn = obj_wrapper.obj_next;

		for(int i = 0; i < float_v::Size; ++i)
			world[(int)objn.pos.x[i]][(int)objn.pos.y[i]] += paint_val;
	}
}

INLINE void Objs2UpdateCurrent(Objects2& objs2, OObject2UpdateCurrent) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	for (auto& obj_wrapper : objs2)
		obj_wrapper.obj = obj_wrapper.obj_next;
}

