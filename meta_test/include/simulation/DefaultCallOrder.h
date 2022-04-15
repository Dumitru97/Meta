#pragma once
using namespace FN;

void DefaultCallOrder1() {
	Actors1Update(Actors1_gv, {});

	Objs1MovedByActs1(Objects1_gv, Actors1_gv, {}, {});
	Objs1WorldWrap(Objects1_gv, {}, {});
	Objs1Draw(Objects1_gv, {});
	Objs1UpdateCurrent(Objects1_gv, {});

	Objs2MovedByActs1(Objects2_gv, Actors1_gv, {}, {});
	Objs2WorldWrap(Objects2_gv, {}, {});
	Objs2Draw(Objects2_gv, {});
	Objs2UpdateCurrent(Objects2_gv, {});

}

void DefaultCallOrder2() {
	Actors1Update(Actors1_gv, {});

	Objs1MovedByActs1(Objects1_gv, Actors1_gv, {}, {});

	Objs2MovedByActs1(Objects2_gv, Actors1_gv, {}, {});


	Objs1WorldWrap(Objects1_gv, {}, {});
	Objs2WorldWrap(Objects2_gv, {}, {});

	Objs1Draw(Objects1_gv, {});
	Objs2Draw(Objects2_gv, {});

	Objs1UpdateCurrent(Objects1_gv, {});
	Objs2UpdateCurrent(Objects2_gv, {});
}

