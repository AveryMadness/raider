#pragma once

#include "SDK.hpp"
#include "game.h"
#include "hooks.h"
#include "ufunctionhooks.h"
#include "Logger.hpp"

DWORD WINAPI Main(LPVOID lpParam)
{
    
    SetupConsole();
    
    auto Start = std::chrono::steady_clock::now();
    Native::InitializeAll();
    auto End = std::chrono::steady_clock::now();

    raider::utils::Logger::Initialize();

    LOG_INFO("Welcome to Raider!");
    LOG_INFO("Initializing hooks!");
    UObject::FindObject<UFunction>("Function FortniteGame.FortKismetLibrary.GetAllFortPlayerPawns");
    UObject::FindObject<UFunction>("Function FortniteGame.FortGameStateAthena.OnRep_WinningPlayerName");
    UObject::FindObject<UFunction>("Function FortniteGame.FortPlayerControllerAthena.PlayWinEffects");
    UObject::FindObject<UFunction>("Function FortniteGame.FortPlayerControllerAthena.ClientNotifyWon");
    UObject::FindObject<UFunction>("Function Engine.PlayerController.ClientGameEnded");
    UObject::FindObject<UFunction>("Function Engine.GameMode.ReadyToEndMatch");
    UObject::FindObject<UFunction>("Function Engine.GameMode.EndMatch");
    TArray<AFortPlayerPawn*> PlayerPawns;
    GetFortKismet()->STATIC_GetAllFortPlayerPawns(GetWorld(), &PlayerPawns);

    UFunctionHooks::Initialize();

    DETOUR_START
    DetourAttachE(Native::NetDriver::TickFlush, Hooks::TickFlush);
    DetourAttachE(Native::LocalPlayer::SpawnPlayActor, Hooks::LocalPlayerSpawnPlayActor);

    auto Address = Utils::FindPattern(Patterns::NetDebug);
    CheckNullFatal(Address, "Failed to find NetDebug");
    void* (*NetDebug)(void* _this) = nullptr;
    AddressToFunction(Address, NetDebug);

    DetourAttachE(NetDebug, Hooks::NetDebug);
    DetourAttachE(ProcessEvent, Hooks::ProcessEventHook);
    DETOUR_END

	Globals::MathLibrary = UObject::FindObject<UKismetMathLibrary>("Default__KismetMathLibrary");
    if (Globals::MathLibrary && Globals::MathLibrary->IsA(UKismetMathLibrary::StaticClass()))
            LOG_INFO("KismetMathLibrary Found.");

    Globals::GameplayStatics = UObject::FindObject<UGameplayStatics>("Default__GameplayStatics");
    if (Globals::GameplayStatics && Globals::GameplayStatics->IsA(UGameplayStatics::StaticClass()))
        LOG_INFO("GameplayStatics Found.");
    

    //printf("[+] Hooked ProcessEvent");
    LOG_INFO("Hooked ProcessEvent");

    LOG_INFO("Base Address: {:X}", Offsets::Imagebase);	
    CreateConsole();

	
	
	
    return 1;
	
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
        CreateThread(0, 0, Main, hModule, 0, 0);

    return true;
}
