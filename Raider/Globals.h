#pragma once
#include "SDK.hpp";
using namespace SDK;

namespace Globals
{
    UKismetMathLibrary* MathLibrary;
    UGameplayStatics* GameplayStatics;
    UFortEngine* FortEngine;
    bool bSoloGame;
    bool bPlayground;
    bool bIgnoreSoloAndPlaygroundBools;
    bool bRespawnPlayers;
    bool bDBNO;
    bool bLargeTeamGame;
    bool bFriendlyFire;
    bool bLateGame;
    float MaxHealth;
    float MaxShield;
    bool DebugRespawn;

	// EXPERMIMENTAL - THESE MAY BREAK INVENTORY, ETC
    bool bSpawnConsumables = false;
    bool bSpawnTraps = false;

}
