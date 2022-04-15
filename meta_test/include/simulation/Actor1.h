#pragma once
using Actors1 = Actors;

using namespace ON;

INLINE void Actors1Update(Actors1& acts1, OActor1Update) {
#ifdef PRINT_FUNC
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif
	constexpr float x_adv = -8.974261f;
	for (auto& act : acts1) {
		act.pos.x += x_adv;
	}
}

