#pragma once

#include <functional>

#include "game.h"
#include "replication.h"
#include "Logger.hpp"


// #define LOGGING
#define CHEATS
#define MAXPLAYERS 100

//Define the hook with ufunction full name
//Return true in the lambda to prevent the original function call

namespace UFunctionHooks
{

    typedef struct MyData
    {
        AFortPlayerControllerAthena* DeadPC;
        AFortPlayerStateAthena* DeadPlayerState;
        AFortPlayerPawnAthena* DeadPawn;
    } MYDATA, *PMYDATA;

	
	
   

    inline std::vector<UFunction*> toHook;
    inline std::vector<std::function<bool(UObject*, void*)>> toCall;

    #define DEFINE_PEHOOK(ufunctionName, func)                           \
        toHook.push_back(UObject::FindObject<UFunction>(ufunctionName)); \
        toCall.push_back([](UObject * Object, void* Parameters) -> bool func);

    auto Initialize()
    {
        DEFINE_PEHOOK("Function GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbility", {
        auto AbilitySystemComponent = (UAbilitySystemComponent*)Object;
        auto Params = (UAbilitySystemComponent_ServerTryActivateAbility_Params*)Parameters;

        TryActivateAbility(AbilitySystemComponent, Params->AbilityToActivate, Params->InputPressed, &Params->PredictionKey, nullptr);

        return false;
        })

        DEFINE_PEHOOK("Function GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbilityWithEventData", {
        auto AbilitySystemComponent = (UAbilitySystemComponent*)Object;
        auto Params = (UAbilitySystemComponent_ServerTryActivateAbilityWithEventData_Params*)Parameters;

        TryActivateAbility(AbilitySystemComponent, Params->AbilityToActivate, Params->InputPressed, &Params->PredictionKey, &Params->TriggerEventData);

        return false;
        })

        DEFINE_PEHOOK("Function GameplayAbilities.AbilitySystemComponent.ServerAbilityRPCBatch", {
        auto AbilitySystemComponent = (UAbilitySystemComponent*)Object;
        auto Params = (UAbilitySystemComponent_ServerAbilityRPCBatch_Params*)Parameters;

        TryActivateAbility(AbilitySystemComponent, Params->BatchInfo.AbilitySpecHandle, Params->BatchInfo.InputPressed, &Params->BatchInfo.PredictionKey, nullptr);

        return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerPawn.ServerHandlePickup", {
        Inventory::OnPickup((AFortPlayerControllerAthena*)((APawn*)Object)->Controller, Parameters);
        return false;
        })

#ifdef CHEATS
        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerCheat", {
        if (Object->IsA(AFortPlayerControllerAthena::StaticClass()))
        {
            auto PC = (AFortPlayerControllerAthena*)Object;
            auto Params = (AFortPlayerController_ServerCheat_Params*)Parameters;

            if (Params && PC && !PC->IsInAircraft())
            {
                auto Pawn = (APlayerPawn_Athena_C*)PC->Pawn;
                auto Message = Params->Msg.ToString() + ' ';

                std::vector<std::string> Arguments;

                while (Message.find(" ") != -1)
                {
                    Arguments.push_back(Message.substr(0, Message.find(' ')));
                    Message.erase(0, Message.find(' ') + 1);
                }

                auto NumArgs = Arguments.size() - 1;

                if (NumArgs >= 0)
                {
                    auto& Command = Arguments[0]; // TODO: Make the string all lower case.
                    std::transform(Command.begin(), Command.end(), Command.begin(), ::tolower);

                    if (Command == "setpickaxe" && NumArgs >= 1)
                    {
                        auto& PickaxeName = Arguments[1];
                        UFortWeaponMeleeItemDefinition* PID = UObject::FindObject<UFortWeaponMeleeItemDefinition>("WID_Harvest_" + PickaxeName + "_Athena_C_T01" + ".WID_Harvest_" + PickaxeName + "_Athena_C_T01");

                        // if (!PID)
                        // PID = UObject::FindObject<UFortWeaponMeleeItemDefinition>(PickaxeName + "." + PickaxeName); // UAthenaPickaxeItemDefinition

                        if (PID && (PID->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) || PID->IsA(UAthenaPickaxeItemDefinition::StaticClass())))
                        {
                            bool bFound = false;
                            auto PickaxeEntry = FindItemInInventory<UFortWeaponMeleeItemDefinition>(PC, bFound);

                            if (!bFound)
                                PickaxeEntry = FindItemInInventory<UAthenaPickaxeItemDefinition>(PC, bFound);

                            if (bFound)
                            {
                                // ChangeItem(PC, PickaxeEntry.ItemDefinition, PID, 0);
                                ClientMessage(PC, (L"Changed pickaxe to " + toWStr(PickaxeName) + L"!").c_str());
                            }

                            else
                                ClientMessage(PC, L"Unable to find your pickaxe!\n");
                        }
                        else
                            ClientMessage(PC, L"Requested item is not a pickaxe!\n");
                    }

                    else if (Command == "setcid" && NumArgs >= 1)
                    {
                    }

                    else if (Command == "equiptraptool")
                    {
                        EquipTrapTool(PC);
                    }

                    else if (Command == "revive" && Pawn->bIsDBNO)
                    {
                        Pawn->bIsDBNO = false;
                        Pawn->OnRep_IsDBNO();

                        // PC->ClientOnPawnRevived(InstigatorPC);
                        Pawn->SetHealth(100);
                    }

                    else if (Command == "togglegodmode")
                    {
                        Pawn->bCanBeDamaged = !Pawn->bCanBeDamaged;
                        std::cout << "Godmode enabled for " << Pawn->GetName() << '\n';
                    }

                    else if (Command == "giveweapon" && NumArgs >= 3)
                    {
                        auto& weaponName = Arguments[1];
                        auto& playerName = Arguments[2];
                        auto& slota = Arguments[3];
                        int slot = std::stoi(slota);
                        int count = 1;
                        auto GameplayStatics = UObject::FindObject<UGameplayStatics>("Default__GameplayStatics");
                        TArray<AActor*> Players;
                        GameplayStatics->STATIC_GetAllActorsOfClass(GetWorld(), AFortPlayerControllerAthena::StaticClass(), &Players);
                        for (int i = 0; i < Players.Num(); i++)
                        {
                            auto Player = (AFortPlayerControllerAthena*)Players[i];
                            if (Player->PlayerState->PlayerName.ToString() == playerName)
                            {
                                std::cout << "Player Found!" << std::endl;
                                auto WID = UObject::FindObject<UFortWeaponRangedItemDefinition>("FortWeaponRangedItemDefinition " + weaponName + '.' + weaponName);

                                if (WID && WID->IsA(UFortWeaponRangedItemDefinition::StaticClass()))
                                {
                                    if (Player)
                                    {
                                        AddItemWithUpdate(Player, WID, slot, EFortQuickBars::Primary, count);
                                        ClientMessage(PC, std::wstring(L"Successfully gave " + count + std::wstring(L" ") + toWStr(weaponName) + L" to slot " + std::to_wstring(slot)).c_str());
                                    }
                                }
                                else
                                    ClientMessage(PC, L"Requested item is not a weapon!\n");
                            }
                            else
                                continue;
                        }
                    }

                    else
                        ClientMessage(PC, L"Unable to handle command!");
                }
            }
        }

        return false;
        })
#endif

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerCreateBuildingActor", {
        auto PC = (AFortPlayerControllerAthena*)Object;
            auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;

        auto Params = (AFortPlayerController_ServerCreateBuildingActor_Params*)Parameters;
        auto CurrentBuildClass = Params->BuildingClassData.BuildingClass;

        static auto GameState = reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState);

        if (PC && Params && CurrentBuildClass)
        {
            {
                if (Globals::bRespawnPlayers || ((AAthena_GameState_C*)GetWorld()->AuthorityGameMode->GameState)->GamePhase == EAthenaGamePhase::Warmup)
                {
                    auto BuildingActor = (ABuildingSMActor*)SpawnActor(CurrentBuildClass, Params->BuildLoc, Params->BuildRot, PC, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
                    // SpawnBuilding(CurrentBuildClass, Params->BuildLoc, Params->BuildRot, (APlayerPawn_Athena_C*)PC->Pawn);
                    if (BuildingActor && CanBuild2(BuildingActor))
                    {
                        // Buildings.insert(BuildingActor); // Add as soon as possible to make sure there is no time to double build.

                        BuildingActor->DynamicBuildingPlacementType = EDynamicBuildingPlacementType::DestroyAnythingThatCollides;
                        BuildingActor->SetMirrored(Params->bMirrored);
                        // BuildingActor->PlacedByPlacementTool();
                        BuildingActor->InitializeKismetSpawnedBuildingActor(BuildingActor, PC);
                        BuildingActor->Team = PlayerState->TeamIndex;
                    }
                    else
                    {
                        BuildingActor->SetActorScale3D({});
                        BuildingActor->SilentDie();
                    }
                    return false;
                }
                if (RemoveBuildingAmount(CurrentBuildClass, PC) && !Globals::bRespawnPlayers)
                {
                    auto BuildingActor = (ABuildingSMActor*)SpawnActor(CurrentBuildClass, Params->BuildLoc, Params->BuildRot, PC, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
                    // SpawnBuilding(CurrentBuildClass, Params->BuildLoc, Params->BuildRot, (APlayerPawn_Athena_C*)PC->Pawn);
                    if (BuildingActor && CanBuild2(BuildingActor))
                    {
                        // Buildings.insert(BuildingActor); // Add as soon as possible to make sure there is no time to double build.

                        BuildingActor->DynamicBuildingPlacementType = EDynamicBuildingPlacementType::DestroyAnythingThatCollides;
                        BuildingActor->SetMirrored(Params->bMirrored);
                        // BuildingActor->PlacedByPlacementTool();
                        BuildingActor->InitializeKismetSpawnedBuildingActor(BuildingActor, PC);
                        BuildingActor->Team = PlayerState->TeamIndex;
                    }
                    else
                    {
                        BuildingActor->SetActorScale3D({});
                        BuildingActor->SilentDie();
                    }
                }
            }
        }

        return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerBeginEditingBuildingActor", {
        auto Params = (AFortPlayerController_ServerBeginEditingBuildingActor_Params*)Parameters;
        auto Controller = (AFortPlayerControllerAthena*)Object;
        auto Pawn = (APlayerPawn_Athena_C*)Controller->Pawn;
        bool bFound = false;
        auto EditToolEntry = FindItemInInventory<UFortEditToolItemDefinition>(Controller, bFound);

        if (Controller && Pawn && Params->BuildingActorToEdit && bFound)
        {
            auto EditTool = (AFortWeap_EditingTool*)EquipWeaponDefinition(Pawn, (UFortWeaponItemDefinition*)EditToolEntry.ItemDefinition, EditToolEntry.ItemGuid);

            if (EditTool)
            {
                EditTool->EditActor = Params->BuildingActorToEdit;
                EditTool->OnRep_EditActor();
                Params->BuildingActorToEdit->EditingPlayer = (AFortPlayerStateZone*)Pawn->PlayerState;
                Params->BuildingActorToEdit->OnRep_EditingPlayer();
            }
        }

        return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortDecoTool.ServerSpawnDeco", {
        SpawnDeco((AFortDecoTool*)Object, Parameters);
        return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerEditBuildingActor", {
        auto Params = (AFortPlayerController_ServerEditBuildingActor_Params*)Parameters;
        auto PC = (AFortPlayerControllerAthena*)Object;
        auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;

        if (PC && Params)
        {
            auto BuildingActor = Params->BuildingActorToEdit;
            auto NewBuildingClass = Params->NewBuildingClass;
            auto RotationIterations = Params->RotationIterations;

            if (BuildingActor && NewBuildingClass)
            {
                auto location = BuildingActor->K2_GetActorLocation();
                auto rotation = BuildingActor->K2_GetActorRotation();

                int yaw = (int(rotation.Yaw) + 360) % 360; // Gets the rotation ranging from 0 to 360 degrees

                if (BuildingActor->BuildingType != EFortBuildingType::Wall) // Centers building pieces if necessary
                {
                    switch (yaw)
                    {
                    case 89:
                    case 90:
                    case 91: // Sometimes the rotation may differ by 1
                        switch (RotationIterations)
                        {
                        case 1:
                            location.X += -256;
                            location.Y += 256;
                            break;
                        case 2:
                            location.X += -512;
                            location.Y += 0;
                            break;
                        case 3:
                            location.X += -256;
                            location.Y += -256;
                            break;
                        }
                        yaw = 90;
                        break;
                    case 179:
                    case 180:
                    case 181:
                        switch (RotationIterations)
                        {
                        case 1:
                            location.X += -256;
                            location.Y += -256;
                            break;
                        case 2:
                            location.X += 0;
                            location.Y += -512;
                            break;
                        case 3:
                            location.X += 256;
                            location.Y += -256;
                            break;
                        }
                        yaw = 180;
                        break;
                    case 269:
                    case 270:
                    case 271:
                        switch (RotationIterations)
                        {
                        case 1:
                            location.X += 256;
                            location.Y += -256;
                            break;
                        case 2:
                            location.X += 512;
                            location.Y += 0;
                            break;
                        case 3:
                            location.X += 256;
                            location.Y += 256;
                            break;
                        }
                        yaw = 270;
                        break;
                    default: // 0, 360. etc.
                        switch (RotationIterations)
                        {
                        case 1:
                            location.X += 256;
                            location.Y += 256;
                            break;
                        case 2:
                            location.X += 0;
                            location.Y += 512;
                            break;
                        case 3:
                            location.X += -256;
                            location.Y += 256;
                            break;
                        }
                        yaw = 0;
                    }
                }

                rotation.Yaw = yaw + 90 * RotationIterations;

                auto HealthPercent = BuildingActor->GetHealthPercent();

                //  BuildingActor->K2_DestroyActor();
                BuildingActor->SilentDie();

                if (auto NewBuildingActor = (ABuildingSMActor*)SpawnActor(NewBuildingClass, location, rotation, PC))
                {
                    if (!BuildingActor->bIsInitiallyBuilding)
                        NewBuildingActor->ForceBuildingHealth(NewBuildingActor->GetMaxHealth() * HealthPercent);
                    NewBuildingActor->SetMirrored(Params->bMirrored);
                    NewBuildingActor->Team = PlayerState->TeamIndex;
                    NewBuildingActor->InitializeKismetSpawnedBuildingActor(NewBuildingActor, PC);
                }
            }
        }

        return false;
        })

        

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerControllerZone.ClientOnPawnDied", {
        auto CurrentParams = (AFortPlayerControllerZone_ClientOnPawnDied_Params*)Parameters;
        auto DeadPC = (AFortPlayerControllerAthena*)Object;
        auto DeadPlayerState = (AFortPlayerStateAthena*)DeadPC->PlayerState;
        auto DeadPawn = (APlayerPawn_Athena_C*)DeadPC->Pawn;
        auto GameMode = (AAthena_GameMode_C*)GetWorld()->AuthorityGameMode;
        if (DeadPC->NetConnection)
        {

            FDeathInfo DeathData;
            DeathData.bDBNO = false;
            DeathData.DeathLocation = DeadPC->Pawn->K2_GetActorLocation();
            DeathData.Distance = CurrentParams->DeathReport.KillerPawn ? CurrentParams->DeathReport.KillerPawn->GetDistanceTo(DeadPC->Pawn) : 0;
            DeathData.DeathCause = Utils::GetDeathCause(CurrentParams->DeathReport);
            DeathData.FinisherOrDowner = (AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState ? (AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState : DeadPlayerState;
            DeadPlayerState->DeathInfo = DeathData;
            DeadPlayerState->OnRep_DeathInfo();
            if (DeadPC && CurrentParams->DeathReport.KillerPlayerState && DeadPlayerState != CurrentParams->DeathReport.KillerPlayerState)
            {
                ((AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState)->ClientReportKill((AFortPlayerStateAthena*)DeadPC->PlayerState);
                ((AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState)->KillScore++;
                ((AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState)->TeamKillScore++;
                ((AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState)->OnRep_Kills();
                ((AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState)->OnRep_Score();
            }

            if (Globals::bRespawnPlayers || Globals::DebugRespawn)
            {
                auto SpawnLoc = DeadPC->Pawn->K2_GetActorLocation();
                DeadPawn->K2_DestroyActor();
                InitPawn(DeadPC, FVector(SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z + 5000), FQuat(), false, false);
                DeadPawn->SetHealth(100);
                DeadPawn->CharacterMovement->SetMovementMode(EMovementMode::MOVE_Custom, 3U);
                bool bFound = false;
                auto PickaxeEntry = FindItemInInventory<UFortWeaponMeleeItemDefinition>(DeadPC, bFound);
                if (bFound)
                    EquipInventoryItem(DeadPC, PickaxeEntry.ItemGuid);
                auto CastedManager = (UFortCheatManager*)DeadPC->CheatManager;
                DeadPC->RespawnPlayerAfterDeath();
                CastedManager->RespawnPlayer();
                CastedManager->RespawnPlayerServer();
                return false;
            }

            for (int i = 0; i < DeadPC->WorldInventory->Inventory.ItemInstances.Num(); i++)
            {
                auto ItemInstance = DeadPC->WorldInventory->Inventory.ItemInstances[i];
                auto ItemDef = ItemInstance->ItemEntry.ItemDefinition;
                auto Count = ItemInstance->ItemEntry.Count;
                if (ItemDef->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) || ItemDef->IsA(UFortBuildingItemDefinition::StaticClass()) || ItemDef->IsA(UFortEditToolItemDefinition::StaticClass()))
                    continue;
                if (!ItemInstance || !ItemDef || !DeadPC->Pawn)
                    continue;

                SummonPickup((AFortPlayerPawnAthena*)DeadPC->Pawn, ItemDef, Count, DeadPC->Pawn->K2_GetActorLocation());
            }
            DeadPC->Pawn->K2_DestroyActor();
            LOG_INFO("Dropped Dead Players Items!")

            auto GameState = reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState);
            GameState->PlayersLeft--;
            GameState->TotalPlayers--;
            if (GameState->PlayersLeft != GameState->TotalPlayers)
                GameState->PlayersLeft = GameState->TotalPlayers;
            LOG_INFO("Subtracted Player Count!")
            GameState->OnRep_PlayersLeft();
            if (((AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState))
                Spectate(DeadPC->NetConnection, ((AFortPlayerStateAthena*)CurrentParams->DeathReport.KillerPlayerState));
            if (GameState->PlayersLeft == 1)
            {
                TArray<AFortPlayerPawn*> OutActors;
                GetFortKismet()->STATIC_GetAllFortPlayerPawns(GetWorld(), &OutActors);

                auto Winner = OutActors[0];
                if (Winner)
                {

                    auto Controller = static_cast<AFortPlayerControllerAthena*>(Winner->Controller);

                    if (!Controller->bClientNotifiedOfWin)
                    {
                        GameState->WinningPlayerName = Controller->PlayerState->GetPlayerName();
                        GameState->OnRep_WinningPlayerName();

                        Controller->PlayWinEffects();
                        Controller->ClientNotifyWon();

                        Controller->ClientGameEnded(Winner, true);
                        GameMode->ReadyToEndMatch();
                        GameMode->EndMatch();
                    }
                    LOG_INFO("Clients notified of win.")
                    if (GetWorld() && GetWorld()->NetDriver && GetWorld()->NetDriver->ClientConnections.Data)
                    {
                        auto Connections = HostBeacon->NetDriver->ClientConnections;

                        for (int i = 0; i < Connections.Num(); i++)
                        {
                            auto Controller = (AFortPlayerControllerAthena*)Connections[i]->PlayerController;

                            if (!Controller || !Controller->IsA(AFortPlayerControllerAthena::StaticClass()))
                                continue;

                            if (Controller && !Controller->bClientNotifiedOfWin)
                                Controller->ClientGameEnded(Winner, false);
                        }
                    }
                }
            }

            // CreateThread(0, 0, RespawnPlayerThread, &Params, 0, nullptr);
        }
       

        return false;
        })

		

	     

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerEndEditingBuildingActor", {
        auto Params = (AFortPlayerController_ServerEndEditingBuildingActor_Params*)Parameters;
        auto PC = (AFortPlayerControllerAthena*)Object;

        if (!PC->IsInAircraft() && Params->BuildingActorToStopEditing)
        {
            Params->BuildingActorToStopEditing->EditingPlayer = nullptr;
            Params->BuildingActorToStopEditing->OnRep_EditingPlayer();

            auto EditTool = (AFortWeap_EditingTool*)((APlayerPawn_Athena_C*)PC->Pawn)->CurrentWeapon;

            if (EditTool)
            {
                EditTool->bEditConfirmed = true;
                EditTool->EditActor = nullptr;
                EditTool->OnRep_EditActor();
            }
        }

        return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerRepairBuildingActor", {
        auto Params = (AFortPlayerController_ServerRepairBuildingActor_Params*)Parameters;
        auto Controller = (AFortPlayerControllerAthena*)Object;
        auto Pawn = (APlayerPawn_Athena_C*)Controller->Pawn;

        if (Controller && Pawn && Params->BuildingActorToRepair)
        {
            Params->BuildingActorToRepair->RepairBuilding(Controller, 10); // TODO: Figure out how to get the repair amount
        }

        return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerControllerAthena.ServerAttemptAircraftJump", {
            auto Params = (AFortPlayerControllerAthena_ServerAttemptAircraftJump_Params*)Parameters;
            auto PC = (AFortPlayerControllerAthena*)Object;
            auto GameState = (AAthena_GameState_C*)GetWorld()->AuthorityGameMode->GameState;

            if (PC && Params && !PC->Pawn && PC->IsInAircraft())
            {
                auto Aircraft = (AFortAthenaAircraft*)GameState->Aircrafts[0];

                if (Aircraft)
                {
                    auto ExitLocation = Aircraft->K2_GetActorLocation();

                    // ExitLocation.Z -= 500;

                    InitPawn(PC, ExitLocation, FQuat(), false, false);
                    PlayersJumpedFromBus++;
                    PC->ClientSetRotation(Params->ClientRotation, false);
                    PC->Pawn->bCanBeDamaged = true;
                    ((AAthena_GameState_C*)GetWorld()->AuthorityGameMode->GameState)->Aircrafts[0]->PlayEffectsForPlayerJumped();
                    PC->ActivateSlot(EFortQuickBars::Primary, 0, 0, true); // Select the pickaxe

                    bool bFound = false;
                    auto PickaxeEntry = FindItemInInventory<UFortWeaponMeleeItemDefinition>(PC, bFound);

                    if (bFound)
                        EquipInventoryItem(PC, PickaxeEntry.ItemGuid);
                    if (bLateGame)
                    {
                       
                        FFortItemEntry ItemEntry = AddItem(PC, Utils::GetRandomWeaponDefinition(Utils::AthenaAssaultLootPool), 1, EFortQuickBars::Primary, 1);
                        EquipWeaponDefinition(PC->Pawn, (UFortWeaponItemDefinition*)ItemEntry.ItemDefinition, ItemEntry.ItemGuid, -1, true);
                        AddItem(PC, Utils::GetRandomWeaponDefinition(Utils::AthenaAssaultLootPool)->GetAmmoWorldItemDefinition_BP(), 0, EFortQuickBars::Secondary, 500);
                        ItemEntry = AddItem(PC, Utils::GetRandomWeaponDefinition(Utils::AthenaShotgunLootPool), 2, EFortQuickBars::Primary, 1);
                        EquipWeaponDefinition(PC->Pawn, (UFortWeaponItemDefinition*)ItemEntry.ItemDefinition, ItemEntry.ItemGuid, -1, true);
                        AddItem(PC, Utils::GetRandomWeaponDefinition(Utils::AthenaShotgunLootPool)->GetAmmoWorldItemDefinition_BP(), 0, EFortQuickBars::Secondary, 500);
                        ItemEntry = AddItem(PC, Utils::GetRandomWeaponDefinition(Utils::AthenaSmgLootPool), 3, EFortQuickBars::Primary, 1);
                        EquipWeaponDefinition(PC->Pawn, (UFortWeaponItemDefinition*)ItemEntry.ItemDefinition, ItemEntry.ItemGuid, -1, true);
                        AddItem(PC, Utils::GetRandomWeaponDefinition(Utils::AthenaSmgLootPool)->GetAmmoWorldItemDefinition_BP(), 0, EFortQuickBars::Secondary, 500);
                        AddItem(PC, Utils::GetRandomConsumableItemDefinition(), 4, EFortQuickBars::Primary, 5);
                        AddItem(PC, Utils::GetRandomConsumableItemDefinition(), 5, EFortQuickBars::Primary, 5);
                    }
                    if (PlayersJumpedFromBus >= GameState->PlayerArray.Num() && bLateGame)
                    {
                        ((AFortGameModeAthena*)GetWorld()->AuthorityGameMode)->OnAircraftExitedDropZone(GameState->GetAircraft(0));
                        GameState->SafeZonesStartTime -= 59;
                    }
                    // PC->Pawn->K2_TeleportTo(ExitLocation, Params->ClientRotation);
                }
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerPawn.ServerReviveFromDBNO", {
            auto Params = (AFortPlayerPawn_ServerReviveFromDBNO_Params*)Parameters;
            auto DBNOPawn = (APlayerPawn_Athena_C*)Object;
            auto DBNOPC = (AFortPlayerControllerAthena*)DBNOPawn->Controller;
            auto InstigatorPC = (AFortPlayerControllerAthena*)Params->EventInstigator;

            if (InstigatorPC && DBNOPawn && DBNOPC)
            {
                DBNOPawn->bIsDBNO = false;
                DBNOPawn->OnRep_IsDBNO();

                DBNOPC->ClientOnPawnRevived(InstigatorPC);
                DBNOPawn->SetHealth(100);
                //ApplyAbilities(DBNOPawn);
				
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerAttemptInteract", {
            auto Params = (AFortPlayerController_ServerAttemptInteract_Params*)Parameters;
            auto PC = (AFortPlayerControllerAthena*)Object;

            if (Params->ReceivingActor)
            {
                auto DBNOPawn = (APlayerPawn_Athena_C*)Params->ReceivingActor;
                auto DBNOPC = (AFortPlayerControllerAthena*)DBNOPawn->Controller;

                if (DBNOPawn && DBNOPC && DBNOPawn->IsA(APlayerPawn_Athena_C::StaticClass()))
                {
                    DBNOPawn->ReviveFromDBNO(PC);
                }
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerPlayEmoteItem", {
            if (!Object->IsA(AFortPlayerControllerAthena::StaticClass()))
                return false;

            auto CurrentPC = (AFortPlayerControllerAthena*)Object;
            auto CurrentPawn = (APlayerPawn_Athena_C*)CurrentPC->Pawn;

            auto EmoteParams = (AFortPlayerController_ServerPlayEmoteItem_Params*)Parameters;
            auto AnimInstance = (UFortAnimInstance*)CurrentPawn->Mesh->GetAnimInstance();

            if (CurrentPC && !CurrentPC->IsInAircraft() && CurrentPawn && EmoteParams->EmoteAsset && AnimInstance && !AnimInstance->bIsJumping && !AnimInstance->bIsFalling)
            {
                // ((UFortCheatManager*)CurrentPC->CheatManager)->AthenaEmote(EmoteParams->EmoteAsset->Name.ToWString().c_str());
                // CurrentPC->ServerEmote(EmoteParams->EmoteAsset->Name);
                if (EmoteParams->EmoteAsset->IsA(UAthenaDanceItemDefinition::StaticClass()))
                {
                    if (auto Montage = EmoteParams->EmoteAsset->GetAnimationHardReference(CurrentPawn->CharacterBodyType, CurrentPawn->CharacterGender))
                    {
                        auto& RepAnimMontageInfo = CurrentPawn->RepAnimMontageInfo;
                        auto& ReplayRepAnimMontageInfo = CurrentPawn->ReplayRepAnimMontageInfo;
                        auto& RepCharPartAnimMontageInfo = CurrentPawn->RepCharPartAnimMontageInfo;
                        auto& LocalAnimMontageInfo = CurrentPawn->AbilitySystemComponent->LocalAnimMontageInfo;
                        auto Ability = CurrentPawn->AbilitySystemComponent->AllReplicatedInstancedAbilities[0];

                        const auto Duration = AnimInstance->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::Duration, 0.0f, true);

                        if (Duration > 0.f)
                        {
                            ReplayRepAnimMontageInfo.AnimMontage = Montage;
                            LocalAnimMontageInfo.AnimMontage = Montage;
                            if (Ability)
                            {
                                LocalAnimMontageInfo.AnimatingAbility = Ability;
                            }
                            LocalAnimMontageInfo.PlayBit = 1;

                            RepAnimMontageInfo.AnimMontage = Montage;
                            RepAnimMontageInfo.ForcePlayBit = 1;

                            RepCharPartAnimMontageInfo.PawnMontage = Montage;

                            if (Ability)
                            {
                                Ability->CurrentMontage = Montage;
                            }

                            bool bIsStopped = AnimInstance->Montage_GetIsStopped(Montage);

                            if (!bIsStopped)
                            {
                                RepAnimMontageInfo.PlayRate = AnimInstance->Montage_GetPlayRate(Montage);
                                RepAnimMontageInfo.Position = AnimInstance->Montage_GetPosition(Montage);
                                RepAnimMontageInfo.BlendTime = AnimInstance->Montage_GetBlendTime(Montage);
                            }

                            RepAnimMontageInfo.IsStopped = bIsStopped;
                            RepAnimMontageInfo.NextSectionID = 0;

                            // CurrentPawn->Mesh->SetAnimation(Montage);
                            CurrentPawn->OnRep_ReplicatedMovement();
                            CurrentPawn->OnRep_RepAnimMontageStartSection();
                            CurrentPawn->OnRep_CharPartAnimMontageInfo();
                            CurrentPawn->OnRep_ReplicatedAnimMontage();
                            CurrentPawn->OnRep_ReplayRepAnimMontageInfo();
                            CurrentPawn->ForceNetUpdate();
                        }
                    }
                }
            }
            else
                std::cout << "Emote Params are invalid!" << std::endl;

            return false;
        })

        

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerAttemptInventoryDrop", {
            auto PC = (AFortPlayerControllerAthena*)Object;

            if (PC && !PC->IsInAircraft())
                Inventory::OnDrop(PC, Parameters);

            return false;
        })

            DEFINE_PEHOOK("Function FortniteGame.FortPlayerPawn.ServerHandlePickup", {

            auto Pawn = (AFortPlayerPawnAthena*)Object;
            auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
            auto CurrentParams = (AFortPlayerPawn_ServerHandlePickup_Params*)Parameters;

            Inventory::OnPickup(PlayerController, CurrentParams); 
             return false;
            })

        DEFINE_PEHOOK("Function BP_VictoryDrone.BP_VictoryDrone_C.OnSpawnOutAnimEnded", {
            if (Object->IsA(ABP_VictoryDrone_C::StaticClass()))
            {
                auto Drone = (ABP_VictoryDrone_C*)Object;

                if (Drone)
                {
                    Drone->K2_DestroyActor();
                }
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerExecuteInventoryItem", {
            EquipInventoryItem((AFortPlayerControllerAthena*)Object, *(FGuid*)Parameters);

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerReturnToMainMenu", {
            ((AFortPlayerController*)Object)->ClientTravel(L"Frontend", ETravelType::TRAVEL_Absolute, false, FGuid());

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerLoadingScreenDropped", {
            auto Pawn = (APlayerPawn_Athena_C*)((AFortPlayerController*)Object)->Pawn;

            if (Pawn && Pawn->AbilitySystemComponent)
            {
                ApplyAbilities(Pawn);
            }

			//SummonWarmupFloorLoot();
            

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerPawn.ServerChoosePart", {
            auto Params = (AFortPlayerPawn_ServerChoosePart_Params*)Parameters;
            auto Pawn = (APlayerPawn_Athena_C*)Object;

            if (Params && Pawn)
            {
                if (!Params->ChosenCharacterPart && Params->Part == EFortCustomPartType::Body)
                    return true;
            }

            return false;
        })

        DEFINE_PEHOOK("Function Engine.GameMode.ReadyToStartMatch", {
            if (!bListening)
            {
                Game::OnReadyToStartMatch();

                HostBeacon = SpawnActor<AFortOnlineBeaconHost>();
                HostBeacon->ListenPort = 7777;
                auto bInitBeacon = Native::OnlineBeaconHost::InitHost(HostBeacon);
                CheckNullFatal(bInitBeacon, "Failed to initialize the Beacon!");

                HostBeacon->NetDriverName = FName(282); // REGISTER_NAME(282,GameNetDriver)
                HostBeacon->NetDriver->NetDriverName = FName(282); // REGISTER_NAME(282,GameNetDriver)
                HostBeacon->NetDriver->World = GetWorld();

                GetWorld()->NetDriver = HostBeacon->NetDriver;
                GetWorld()->LevelCollections[0].NetDriver = HostBeacon->NetDriver;
                GetWorld()->LevelCollections[1].NetDriver = HostBeacon->NetDriver;

                // Native::OnlineBeacon::PauseBeaconRequests(HostBeacon, false);

                CreateThread(0, 0, MapLoadThread, 0, 0, 0);

                auto GameState = (AAthena_GameState_C*)GetWorld()->GameState;

                // GameState->SpectatorClass = ABP_SpectatorPawn_C::StaticClass();
                // sGameState->OnRep_SpectatorClass();

                ((AAthena_GameMode_C*)GetWorld()->AuthorityGameMode)->GameSession->MaxPlayers = MAXPLAYERS;
                bListening = true;
                std::cout << "\n\nListening on port " << HostBeacon->ListenPort << "\n\n";
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortGameModeAthena.OnAircraftExitedDropZone", { // To make this faster we could loop through client connections and get their controllers

			if (GetWorld() && GetWorld()->NetDriver && GetWorld()->NetDriver->ClientConnections.Data)
            {
                auto Connections = HostBeacon->NetDriver->ClientConnections;

                for (int i = 0; i < Connections.Num(); i++)
                {
                    auto Controller = (AFortPlayerControllerAthena*)Connections[i]->PlayerController;

                    if (!Controller || !Controller->IsA(AFortPlayerControllerAthena::StaticClass()) || Controller->PlayerState->bIsSpectator)
                        continue;

                    if (Controller && Controller->IsInAircraft())
                    {

                        Controller->ServerAttemptAircraftJump(FRotator());
                        
                    }
                    
                }            
            }

            return false;
        })
			
            DEFINE_PEHOOK("Function FortniteGame.FortGameModeAthena.OnAircraftEnteredDropZone", {
                if (GetWorld() && GetWorld()->NetDriver && GetWorld()->NetDriver->ClientConnections.Data)
                {
                    auto Connections = HostBeacon->NetDriver->ClientConnections;

                    for (int i = 0; i < Connections.Num(); i++)
                    {
                        auto Controller = (AFortPlayerControllerAthena*)Connections[i]->PlayerController;

                        if (!Controller || !Controller->IsA(AFortPlayerControllerAthena::StaticClass()))
                            continue;
                        Inventory::RemoveItemFromSlot(Controller, 5, EFortQuickBars::Primary, -1);
                        Inventory::RemoveItemFromSlot(Controller, 4, EFortQuickBars::Primary, -1);
                        Inventory::RemoveItemFromSlot(Controller, 3, EFortQuickBars::Primary, -1);
                        Inventory::RemoveItemFromSlot(Controller, 2, EFortQuickBars::Primary, -1);
                        Inventory::RemoveItemFromSlot(Controller, 1, EFortQuickBars::Primary, -1);
                        auto Inventory = Controller->WorldInventory;

                        for (int i = Inventory->Inventory.ItemInstances.Num() - 1; i >= 0; i--)
						{
							auto Item = Inventory->Inventory.ItemInstances[i];
							if (Item->ItemEntry.ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) || Item->ItemEntry.ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()))
							{
								Inventory::RemoveItemFromSlot(Controller, i, EFortQuickBars::Secondary);
                                Inventory::Update(Controller, 0, true);
							}
						}
                        
                    }
                }

                return false;
            })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerCheatAll", {
            KickController((AFortPlayerControllerAthena*)Object, L"Please do not do that!");
            return true;
        })

        LOG_INFO("[+] Hooked %zu UFunction(s)\n", toHook.size());
    }
}