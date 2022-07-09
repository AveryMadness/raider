// Fortnite (3.1) SDK

#ifdef _MSC_VER
	#pragma pack(push, 0x8)
#endif

#include "../SDK.hpp"

namespace SDK
{
//---------------------------------------------------------------------------
//Functions
//---------------------------------------------------------------------------

// Function BP_VictoryDrone.BP_VictoryDrone_C.NotifyTeleportFinishedTriggered
// (Public, BlueprintCallable, BlueprintEvent)

void ABP_VictoryDrone_C::NotifyTeleportFinishedTriggered()
{
    static auto fn = UObject::FindObject<UFunction>("Function BP_VictoryDrone.BP_VictoryDrone_C.NotifyTeleportFinishedTriggered");

    ABP_VictoryDrone_C_NotifyTeleportFinishedTriggered_Params params;

    auto flags = fn->FunctionFlags;

    UObject::ProcessEvent(fn, &params);

    fn->FunctionFlags = flags;
}

// Function BP_VictoryDrone.BP_VictoryDrone_C.PlaySpawnOutAnim
// (Public, BlueprintCallable, BlueprintEvent)

void ABP_VictoryDrone_C::PlaySpawnOutAnim()
{
    static auto fn = UObject::FindObject<UFunction>("Function BP_VictoryDrone.BP_VictoryDrone_C.PlaySpawnOutAnim");

    ABP_VictoryDrone_C_PlaySpawnOutAnim_Params params;

    auto flags = fn->FunctionFlags;

    UObject::ProcessEvent(fn, &params);

    fn->FunctionFlags = flags;
}


// Function BP_VictoryDrone.BP_VictoryDrone_C.TriggerPlayerSpawnEffects
// (Public, BlueprintCallable, BlueprintEvent)

void ABP_VictoryDrone_C::TriggerPlayerSpawnEffects()
{
    static auto fn = UObject::FindObject<UFunction>("Function BP_VictoryDrone.BP_VictoryDrone_C.TriggerPlayerSpawnEffects");

    ABP_VictoryDrone_C_TriggerPlayerSpawnEffects_Params params;

    auto flags = fn->FunctionFlags;

    UObject::ProcessEvent(fn, &params);

    fn->FunctionFlags = flags;
}


// Function BP_VictoryDrone.BP_VictoryDrone_C.InitDrone
// (Public, BlueprintCallable, BlueprintEvent)

void ABP_VictoryDrone_C::InitDrone()
{
    static auto fn = UObject::FindObject<UFunction>("Function BP_VictoryDrone.BP_VictoryDrone_C.InitDrone");

    ABP_VictoryDrone_C_InitDrone_Params params;

    auto flags = fn->FunctionFlags;

    UObject::ProcessEvent(fn, &params);

    fn->FunctionFlags = flags;
}


}

#ifdef _MSC_VER
	#pragma pack(pop)
#endif
