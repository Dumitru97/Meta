# Meta

This repository demonstrates and benchmarks an application of C++ reflection and metaprogramming features implemented in `https://github.com/lock3/meta` based on commit `6632fc0a490d0f7ef83021844cd88ef153e38237`.

## meta_lib
Header-only library used in two stages/separate executables:
### Precompute stage:
- Takes as input pairs of namespaces: for `functions` and for `orders` structs
- Extracts data about the functions' parameters and `order tags`
- Runs `simulated annealing` over the functions' data to determined the best call ordering given `order tags` constraints
- Outputs header files containing arrays `indexing function calls`
### Execute stage:
- Takes as input pairs of namespaces: for `functions` and for `orders` structs
- Extracts information about the functions' parameters and order tags
- Creates variables based on functions' parameters' type identifiers to be used as `function arguments`
- `Calls functions` in the given namespace in the `precomputed order` with the created arguments

### Equivalent conventional workflow(global namespace):
<details>
   <summary> Version1 </summary>
  
```
// Function declarations/definitions
  ...
  
Actors1 Actors1_gv;
Actors2 Actors2_gv;
Objects1 Objects1_gv;
Objects2 Objects2_gv;
Objects3 Objects3_gv;
Objects4 Objects4_gv;
  
void DefaultCallOrder1() {
	Actors1Update(Actors1_gv);
	Actors2Update(Actors2_gv);

	Objs1MovedByActs1(Objects1_gv, Actors1_gv);
	Objs1MovedByActs2(Objects1_gv, Actors2_gv);
	Objs1WorldWrap(Objects1_gv);
	Objs1Draw(Objects1_gv;
	Objs1UpdateCurrent(Objects1_gv);

	Objs2MovedByActs1(Objects2_gv, Actors1_gv);
	Objs2WorldWrap(Objects2_gv);
	Objs2Draw(Objects2_gv);
	Objs2UpdateCurrent(Objects2_gv);

	Objs3MovedByActs2(Objects3_gv, Actors2_gv);
	Objs3WorldWrap(Objects3_gv);
	Objs3Draw(Objects3_gv);
	Objs3UpdateCurrent(Objects3_gv);

	Objs4MovedByActs1(Objects4_gv, Actors1_gv);
	Objs4MovedByActs2(Objects4_gv, Actors2_gv);
	Objs4WorldWrap(Objects4_gv);
	Objs4Draw(Objects4_gv);
	Objs4UpdateCurrent(Objects4_gv);
}
```
</details>

<details>   
  <summary> Version2 </summary>
  
```
// Function declarations/definitions
  ...
  
Actors1 Actors1_gv;
Actors2 Actors2_gv;
Objects1 Objects1_gv;
Objects2 Objects2_gv;
Objects3 Objects3_gv;
Objects4 Objects4_gv;
  
void DefaultCallOrder2() {
	Actors1Update(Actors1_gv);
	Actors2Update(Actors2_gv);

	Objs1MovedByActs1(Objects1_gv, Actors1_gv);
	Objs1MovedByActs2(Objects1_gv, Actors2_gv);

	Objs2MovedByActs1(Objects2_gv, Actors1_gv,);

	Objs3MovedByActs2(Objects3_gv, Actors2_gv);

	Objs4MovedByActs1(Objects4_gv, Actors1_gv);
	Objs4MovedByActs2(Objects4_gv, Actors2_gv);


	Objs1WorldWrap(Objects1_gv);
	Objs2WorldWrap(Objects2_gv);
	Objs3WorldWrap(Objects3_gv);
	Objs4WorldWrap(Objects4_gv);

	Objs1Draw(Objects1_gv);
	Objs2Draw(Objects2_gv);
	Objs3Draw(Objects3_gv);
	Objs4Draw(Objects4_gv);

	Objs1UpdateCurrent(Objects1_gv);
	Objs2UpdateCurrent(Objects2_gv);
	Objs3UpdateCurrent(Objects3_gv);
	Objs4UpdateCurrent(Objects4_gv);
}
```
</details>

### New workflow(global namespace injection):

<details>
  <summary> Order namespace: </summary>

```
namespace ON
{
struct OObjectAnyWrap {};
struct OObjectAnyWrapDone : OObjectAnyWrap {};

struct OObject1MovedBy {};
struct OObject1Wrap : OObject1MovedBy {};
struct OObject1UpdateCurrent : OObject1Wrap {};

struct OObject2MovedBy {};
struct OObject2Wrap : OObject2MovedBy {};
struct OObject2UpdateCurrent : OObject2Wrap {};

struct OObject3MovedBy {};
struct OObject3Wrap : OObject3MovedBy {};
struct OObject3UpdateCurrent : OObject3Wrap {};

struct OObject4MovedBy {};
struct OObject4Wrap : OObject4MovedBy {};
struct OObject4UpdateCurrent : OObject4Wrap {};

struct OActor1Update {};
struct OActor1UpdateDone : OActor1Update {};

struct OActor2Update {};
struct OActor2UpdateDone : OActor2Update {};
}
```
</details>

<details>
  <summary> Function namespace: </summary>

```
namespace FN
{
using namespace ON;

void Actors1Update(Actors1&acts1, OActor1Update);
void Actors2Update(Actors2&acts2, OActor2Update);

void Objs1MovedByActs1(Objects1& objs1, const Actors1& acts1, OObject1MovedBy, OActor1UpdateDone);
void Objs1MovedByActs2(Objects1& objs1, const Actors2& acts2, OObject1MovedBy, OActor2UpdateDone);

void Objs1WorldWrap(Objects1& objs1, OObject1Wrap, OObjectAnyWrap);
void Objs1Draw(const Objects1& objs1, OObjectAnyWrapDone);
void Objs1UpdateCurrent(Objects1& objs1, OObject1UpdateCurrent);

...
}
```
</details>


In MetaArguments1.h:
```
#include "meta.h"
// Function declarations/definitions in namespace FN
// Order struct definitions in namespace ON
#include "simulation/TestIncludes.h"

META_PRECOMPUTE_OR_CREATE_ARGUMENTS(ON, FN);
#include META_INCLUDE_IDXS_HEADER(generated, ON, FN)
```


`Precompute stage`
precompute_main.cpp:
```
#include "MetaArguments1.h"

int main() {}
```


`Execute stage`
main.cpp:
```
#include "MetaArguments1.h"
void CallFuncs() {
	META_CALL_FUNCTIONS_OPTIMIZED(ON, FN);
}
```

### Details:
Argument variable names are based on function parameter type identifiers such that
```
void Objs1MovedByActs1(Objects1& objs1, const Actors1& acts1, OObject1MovedBy, OActor1UpdateDone);
void Objs1MovedByActs2(Objects1& objs1, const Actors2& acts2, OObject1MovedBy, OActor2UpdateDone);
```
creates 3 variables: `Objects1_gv`, `Actors1_gv`, `Actors2_gv`. Both functions are called with `Objects1_gv` with a single memory location.


