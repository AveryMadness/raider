#pragma once

#include "ZeroGUI.h"
#include <format>
#include <mutex>

static bool bStartedBus = false;
static bool bSafeZoneEnabled = true;


// constexpr CustomMode Mode = CustomMode::NONE;

namespace GUI
{
    std::mutex mtx;
    void Tick()
    {
        
		
        ZeroGUI::Input::Handle();

        static bool menu_opened = true;

        if (GetAsyncKeyState(VK_F2) & 1)
            menu_opened = !menu_opened;

        static auto pos = FVector2D { 200.f, 250.0f };

        if (ZeroGUI::Window(L"Raider", &pos, FVector2D { 500.0f, 700.0f }, menu_opened))
        {
            if (bListening && HostBeacon)
            {
                static auto GameState = reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState);
                static APlayerState* currentPlayer = nullptr;
                static int currentDevWeaponIndex = 0;
                static int currentRarityWeaponIndex = 0;
                static std::string currentDevWeapon = "";
                

                // This is bad, but works for now.
                if (currentPlayer)
                {
                    if (ZeroGUI::Button(L"<", { 25.0f, 25.0f }))
                    {
                        mtx.lock();
                        currentPlayer = nullptr;
                        mtx.unlock();
                    }

                    ZeroGUI::NextColumn(90.0f);

                    ZeroGUI::Text(std::format(L"Current Player: {}", currentPlayer->GetPlayerName().c_str()).c_str());

                    ZeroGUI::PushNextElementY(25.0f);
                    if (ZeroGUI::Button(L"Respawn", { 60.0f, 25.0f }))
					{
                        auto DeadPC = (AFortPlayerControllerAthena*)currentPlayer->Owner;
                        auto DeadPawn = (AFortPlayerPawnAthena*)DeadPC->Pawn;
                        InitPawn((AFortPlayerControllerAthena*)currentPlayer->Owner, FVector(0, 0, 2900 + 5000), FQuat(), false, true);
                        DeadPawn->CharacterMovement->SetMovementMode(EMovementMode::MOVE_Custom, 3U);
                        bool bFound = false;
                        auto PickaxeEntry = FindItemInInventory<UFortWeaponMeleeItemDefinition>(DeadPC, bFound);
                        if (bFound)
                            EquipInventoryItem(DeadPC, PickaxeEntry.ItemGuid);

                        auto CastedManager = (UFortCheatManager*)DeadPC->CheatManager;
                        DeadPC->RespawnPlayerAfterDeath();
                        CastedManager->RespawnPlayer();
                        CastedManager->RespawnPlayerServer();
                        mtx.lock();
                        currentPlayer = nullptr;
                        mtx.unlock();
					}

					if (ZeroGUI::Button(L"Spawn Husk", { 60.0f, 25.0f }))
                    {
                        auto PC = (AFortPlayerControllerAthena*)currentPlayer->Owner;
						auto Pawn = (AFortPlayerPawnAthena*)PC->Pawn;
                        FVector SpawnLoc;
						SpawnLoc.X = Pawn->K2_GetActorLocation().X + 100.0f;
						SpawnLoc.Z = Pawn->K2_GetActorLocation().Z + 100.0f;
                        SpawnLoc.Y = Pawn->K2_GetActorLocation().Y;
                        FTransform SpawnTransform;
                        SpawnTransform.Rotation = FQuat{};
                        SpawnTransform.Translation = SpawnLoc;
                        SpawnTransform.Scale3D = FVector { 1.0f, 1.0f, 1.0f };

                        auto HuskActor = Utils::FindObjectFast<AActor>("/Game/Characters/Enemies/Husk/Blueprints/HuskPawn.HuskPawn_C");
                        SpawnActorTrans(HuskActor->StaticClass(), SpawnTransform);
                    }
					
                    if (ZeroGUI::Button(L"Kick", { 60.0f, 25.0f }))
                    {
                        KickController((AFortPlayerControllerAthena*)currentPlayer->Owner, L"You have been kicked by the server.");
                        mtx.lock();
                        currentPlayer = nullptr;
                        mtx.unlock();
                    }

                    if (ZeroGUI::Button(L"Anticheat Kick", {60.0f, 25.0f}))
                    {
                        AnticheatKick((AFortPlayerControllerAthena*)currentPlayer->Owner);
                        mtx.lock();
                        currentPlayer = nullptr;
                        mtx.unlock();
                    }
					
                    ZeroGUI::Combobox(L"Dev Weapon", FVector2D { 100.0f, 25.0f }, &currentDevWeaponIndex, L"Troll Launcher", L"Test God Gun", L"Shot God", L"Wanna Gun", L"Gnome Gun", L"Dev Sniper", L"Charge Rocket", L"Nocturno", L"Scythe", NULL);
                    if (ZeroGUI::Button(L"Give Dev Weapon", {120.0f, 25.0f }))
                    {
                        if (currentDevWeaponIndex == 0)
                            currentDevWeapon = "/Game/Items/Weapons/Ranged/WIP/WID_Troll_Launcher_Durability.WID_Troll_Launcher_Durability";
                        else if (currentDevWeaponIndex == 1)
                            currentDevWeapon = "/Game/Items/Weapons/Ranged/WIP/TestGod.TestGod";
                        else if (currentDevWeaponIndex == 2)
                            currentDevWeapon = "/Game/Items/Weapons/Ranged/WIP/ShotGod.ShotGod";
                        else if (currentDevWeaponIndex == 3)
                            currentDevWeapon = "/Game/Items/Weapons/Ranged/WIP/WannaGun.WannaGun";
						else if (currentDevWeaponIndex == 4)
							currentDevWeapon = "/Game/Items/Weapons/Ranged/WIP/GnomeGun2.GnomeGun2";
						else if (currentDevWeaponIndex == 5)
                            currentDevWeapon = "/Game/Items/Weapons/Ranged/WIP/WID_Sniper_Hydraulic_T01.WID_Sniper_Hydraulic_T01";
						else if (currentDevWeaponIndex == 6)
                            currentDevWeapon = "/Game/Items/Weapons/Ranged/WIP/WID_TestChargeRocket.WID_TestChargeRocket";
						else if (currentDevWeaponIndex == 7)
                            currentDevWeapon = "/Game/Items/Weapons/Ranged/Assault/Auto_High/WID_Assault_Auto_Founders_SR_Ore_T05.WID_Assault_Auto_Founders_SR_Ore_T05";
						else if (currentDevWeaponIndex == 8)
                            currentDevWeapon = "/Game/Items/Weapons/Melee/Edged/Scythe_High/WID_Edged_Scythe_SR_Ore_T06.WID_Edged_Scythe_SR_Ore_T06";
						else
							currentDevWeapon = "";
                        auto PC = (AFortPlayerControllerAthena*)currentPlayer->Owner;
                        auto Pawn = (AFortPlayerPawnAthena*)PC->Pawn;
                        auto WeaponDef = Utils::FindObjectFast<UFortWeaponItemDefinition>(currentDevWeapon);
                        auto Pickup = SummonPickup(Pawn, WeaponDef, 1, Pawn->K2_GetActorLocation());
                        mtx.lock();
                        currentPlayer = nullptr;
                        mtx.unlock(); 
                    }
                    ZeroGUI::Combobox(L"Rarity", FVector2D{100.0f, 25.0f}, &currentRarityWeaponIndex, L"Common", L"Uncommon", L"Rare", L"Epic", L"Legendary", NULL);
                    if (ZeroGUI::Button(L"Give Random Weapon", {120.0f, 25.0f}))
                    {
                        auto PC = (AFortPlayerControllerAthena*)currentPlayer->Owner;
                        auto Pawn = (AFortPlayerPawnAthena*)PC->Pawn;
						UFortWeaponItemDefinition* WeaponDef = nullptr;
                        if (currentRarityWeaponIndex == 0)
                            WeaponDef = Utils::GetRandomCommonWeaponDefinition();
						else if (currentRarityWeaponIndex == 1)
                            WeaponDef = Utils::GetRandomUncommonWeaponDefinition();
						else if (currentRarityWeaponIndex == 2)
                            WeaponDef = Utils::GetRandomRareWeaponDefinition();
						else if (currentRarityWeaponIndex == 3)
                            WeaponDef = Utils::GetRandomEpicWeaponDefinition();
						else if (currentRarityWeaponIndex == 4)
                            WeaponDef = Utils::GetRandomGoldWeaponDefinition();
                        SummonPickup(Pawn, WeaponDef, 1, Pawn->K2_GetActorLocation());
                        mtx.lock();
                        currentPlayer = nullptr;
                        mtx.unlock();
                    }
                    if (ZeroGUI::Button(L"Spawn Supply Drop", FVector2D {100.0f, 25.0f}))
                    {
                        auto PC = (AFortPlayerControllerAthena*)currentPlayer->Owner;
						auto Pawn = (AFortPlayerPawnAthena*)PC->Pawn;
                        FTransform SpawnTransform;
                        SpawnTransform.Rotation = FQuat {};
                        SpawnTransform.Scale3D = FVector { 1, 1, 1 };
						SpawnTransform.Translation = Pawn->K2_GetActorLocation();
                        SpawnActorTrans(AFortAthenaSupplyDrop::StaticClass(), SpawnTransform);
                        mtx.lock();
                        currentPlayer = nullptr;
                        mtx.unlock();
												
						
                    }
					
                    
                    
                }
                
                else
                {
                    static int tab = 0;
                    if (ZeroGUI::ButtonTab(L"Main", FVector2D { 110, 25 }, tab == 0))
						tab = 0;
                    if (ZeroGUI::ButtonTab(L"Players", FVector2D { 110, 25 }, tab == 1))
                        tab = 1;
                    if (ZeroGUI::ButtonTab(L"Game", FVector2D { 110, 25 }, tab == 2))
                        tab = 2;

                    ZeroGUI::NextColumn(130.0f);

                    switch (tab)
                    {
                    case 2:
                    {
                        if (!bStartedBus)
                        {
                            if (ZeroGUI::Button(L"Start Bus", FVector2D { 100, 25 }))
                            {

                                GameState->bGameModeWillSkipAircraft = false;
                                GameState->AircraftStartTime = 0;
                                GameState->WarmupCountdownEndTime = 0;
                                bStartedBus = true;

                                GetKismetSystem()->STATIC_ExecuteConsoleCommand(GetWorld(), L"startaircraft", nullptr);
                            }
                        }
                            if (ZeroGUI::Button(L"Set Server Name", FVector2D {100, 25}))
                            {
                                auto KismetStringLibrary = UObject::FindObject<UKismetTextLibrary>("Default__KismetTextLibrary");
                                GetKismetSystem()->STATIC_SetWindowTitle(KismetStringLibrary->STATIC_Conv_StringToText(L"SERVER"));
                            }
                            if (ZeroGUI::Button(L"Start Safe Zone", FVector2D {100, 25}))
                            {
                                GetKismetSystem()->STATIC_ExecuteConsoleCommand(GetWorld(), L"startsafezone", nullptr);
                            }
                            if (ZeroGUI::Button(L"Pause Safe Zone", FVector2D {100, 25}))
                            {
                                GetKismetSystem()->STATIC_ExecuteConsoleCommand(GetWorld(), L"pausesafezone", nullptr);
                            }
                             if (ZeroGUI::Button(L"Restart Server", FVector2D { 100, 25 }))
							{
                                 bRestart = true;
							} 
                            if (ZeroGUI::Button(L"Spawn Floor Loot", FVector2D {100, 25}))
                            {
                                CreateThread(nullptr, 0, SummonFloorLoot, nullptr, 0, nullptr);
                            }
                            if (ZeroGUI::Button(L"Spawn Supply Drop", FVector2D {100, 25}))
                            {
                                auto GameMode = (AAthena_GameMode_C*)GetWorld()->AuthorityGameMode;
                                GameMode->TrySupplyDrop();
                            }
                            if (ZeroGUI::Button(L"Destroy All Pickups", FVector2D {100, 25}))
                            {
                                auto Pickups = GetAllActorsOfClass(AFortPickupAthena::StaticClass());
                                for (int i = 0; i < Pickups.Num(); i++)
								{
									auto Pickup = (AFortPickupAthena*)Pickups[i];
									Pickup->K2_DestroyActor();
								}
                            }
                        
                        break;
                    }
                    case 1:
                    {
                        std::wstring ConnectedPlayers = std::format(L"Connected Players: {}\n", GameState->PlayerArray.Num());

                        ZeroGUI::Text(ConnectedPlayers.c_str());

                        for (auto i = 0; i < GameState->PlayerArray.Num(); i++)
                        {
                            auto PlayerState = GameState->PlayerArray[i];

                            if (ZeroGUI::Button(PlayerState->GetPlayerName().c_str(), { 100, 25 }))
                            {
                                currentPlayer = PlayerState;
                            }
                        }

                        break;
                    }
                    case 0:
                        ZeroGUI::Text(L"\n\n\n\n\nWelcome to Raider! This is a simple menu for the server.\n Raider was created by Samuel, Milxnor, \nKemo, Fischsalat, and others found at \nhttps://github.com/kem0x/raider3.5, \nand Contributed to by AveryMadness. \nThis project is NOT affiliated with Epic Games©", false);
                    }
                    
                }
            }
            else
            {
                // ZeroGUI::Text(L"Waiting for map to load...");
            }
        }

        ZeroGUI::Render();
        // ZeroGUI::Draw_Cursor(menu_opened);
    }
}