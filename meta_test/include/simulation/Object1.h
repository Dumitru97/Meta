#pragma once
using Objects1 = Objects;

using namespace ON;

INLINE void Objs1MovedByActs1(Objects1& objs1, const Actors1& acts1, OObject1MovedBy, OActor1UpdateDone) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	//constexpr float dist_tresh = 1.4579912f;

	for (const auto& act : acts1)
		for (int i = 0; i < objs1.size(); ++i) {
			(void)act;
			//const auto& obj = objs1[i].obj;
			//auto& obj_next = objs1[i].obj_next;

			//if (distsq(act.pos, obj.pos) > dist_tresh * dist_tresh) {
				objs1[i].obj_next.pos.x += 0.16745222f;
				objs1[i].obj_next.pos.y += -0.08734547f;
			//}
		}
}

INLINE void Objs1WorldWrap(Objects1& objs1, OObject1Wrap, OObjectAnyWrap) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	for (auto& obj_wrapper : objs1) {
		auto& objn = obj_wrapper.obj_next;

		objn.pos.x = objn.pos.x.apply(fmodf_width);
		objn.pos.x(Vc::isnegative(objn.pos.x)) += world_width;

		objn.pos.y = objn.pos.y.apply(fmodf_height);
		objn.pos.y(Vc::isnegative(objn.pos.y)) += world_height;
	}
}

INLINE void Objs1Draw(const Objects1& objs1, OObjectAnyWrapDone) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	constexpr int paint_val = 8;

	for (const auto& obj_wrapper : objs1) {
		const auto& objn = obj_wrapper.obj_next;

		for(int i = 0; i < float_v::Size; ++i)
			world[(int)objn.pos.x[i]][(int)objn.pos.y[i]] += paint_val;
	}
}

INLINE void Objs1UpdateCurrent(Objects1& objs1, OObject1UpdateCurrent) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	for (auto& obj_wrapper : objs1)
		obj_wrapper.obj = obj_wrapper.obj_next;
}

