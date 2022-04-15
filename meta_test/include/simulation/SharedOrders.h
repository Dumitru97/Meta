#pragma once
namespace ON {

struct OObjectAnyWrap {};
struct OObjectAnyWrapDone : OObjectAnyWrap {}; 

struct OObject1MovedBy {};
struct OObject1Wrap : OObject1MovedBy {};
struct OObject1UpdateCurrent : OObject1Wrap {};

struct OObject2MovedBy {};
struct OObject2Wrap : OObject2MovedBy {};
struct OObject2UpdateCurrent : OObject2Wrap {};

struct OActor1Update {};
struct OActor1UpdateDone : OActor1Update {}; 

}

