#pragma once

#include "gui.h"
#include "ufunctionhooks.h"

// #define LOGGING

enum class CustomMode
{
    NONE,
	JUGGERNAUT, // Gives the players 500 health and makes you slower.
    LATEGAME, // TODO: You know what late game is.
	LIFESTEAL, // TODO: You know what life steal is, but this might be a stupid idea.
    SPACE // Sets gravity like the moon // BUG: Unfortunately, the gravityscale variable doesn't update for the client, making them rubberband and making it look weird.
};

static int CurrentTeam = 2;
std::map<EFortTeam, bool> teamsmap;

constexpr CustomMode Mode = CustomMode::NONE;

namespace Hooks
{
    bool LocalPlayerSpawnPlayActor(ULocalPlayer* Player, const FString& URL, FString& OutError, UWorld* World) // prevent server's pc from spawning
    {
        if (bTraveled)
            return true;
        else
            return Native::LocalPlayer::SpawnPlayActor(Player, URL, OutError, World);
    }

    uint64 GetNetMode(UWorld* World) // PlayerController::SendClientAdjustment checks if the netmode is not client
    {
        return 2; // ENetMode::NM_ListenServer;
    }

    //credits to the sulfur team i suppose
    void* OnReloadHook(AFortWeapon* a1, int a2)
    {
        auto Pawn = (AFortPlayerPawnAthena*)a1->GetOwner();
        auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;

        bool Successful = true;

        if (a1 && Successful)
        {
            auto AmmoDef = a1->WeaponData->GetAmmoWorldItemDefinition_BP();

            if (!AmmoDef || a1->WeaponData->GetName().contains("TID"))
                AmmoDef = a1->WeaponData;

            auto Inventory = PlayerController->WorldInventory;

            auto ReplicatedEntries = Inventory->Inventory.ReplicatedEntries;
            auto ItemInstances = Inventory->Inventory.ItemInstances;

            for (int i = 0; i < Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                if (Inventory->Inventory.ReplicatedEntries[i].ItemDefinition == AmmoDef)
                {
                    Inventory->Inventory.ReplicatedEntries[i].Count -= a2;
                    Inventory->Inventory.ReplicatedEntries[i].ReplicationKey++;

                    if (Inventory->Inventory.ReplicatedEntries[i].Count <= 0)
                    {
                        Inventory->Inventory.ReplicatedEntries.RemoveSingle(i);

                        for (int j = 0; j < ItemInstances.Num(); j++)
                        {
                            auto ItemInstance = ItemInstances[j];

                            if (ItemInstance && ItemInstance->GetItemDefinitionBP() == AmmoDef)
                            {
                                ItemInstances.RemoveSingle(i);
                                Inventory::Update(PlayerController, 0, true);
                            }
                        }
                    }

                    Inventory::Update(PlayerController, i, false);
                }
            }

            Native::OnReload(a1, a2);
        }
    }

    void TickFlush(UNetDriver* NetDriver, float DeltaSeconds)
    {
        if (!NetDriver)
            return;

        if (NetDriver->IsA(UIpNetDriver::StaticClass()) && NetDriver->ClientConnections.Num() > 0 && NetDriver->ClientConnections[0]->InternalAck == false)
        {
            Replication::ServerReplicateActors(NetDriver);
        }

        Native::NetDriver::TickFlush(NetDriver, DeltaSeconds);
    }

    void WelcomePlayer(UWorld* World, UNetConnection* IncomingConnection)
    {
        Native::World::WelcomePlayer(GetWorld(), IncomingConnection);
    }

    char KickPlayer(__int64 a1, __int64 a2, __int64 a3)
    {
        return 0;
    }

    void World_NotifyControlMessage(UWorld* World, UNetConnection* Connection, uint8 MessageType, void* Bunch)
    {
        Native::World::NotifyControlMessage(GetWorld(), Connection, MessageType, Bunch);
    }


    auto ApplyTeam()
    {
        if (true)
        {
            for (int i = 2; i < 104; i++)
            {
                teamsmap.insert_or_assign((EFortTeam)i, false);
            }
        }

        for (auto team : teamsmap)
        {
            if (team.second)
                continue;

            teamsmap.insert_or_assign(team.first, true);

            return team.first;
        }
    }

    APlayerController* SpawnPlayActor(UWorld* World, UPlayer* NewPlayer, ENetRole RemoteRole, FURL& URL, void* UniqueId, SDK::FString& Error, uint8 NetPlayerIndex)
    {
        auto PlayerController = (AFortPlayerControllerAthena*)Native::World::SpawnPlayActor(GetWorld(), NewPlayer, RemoteRole, URL, UniqueId, Error, NetPlayerIndex);
        NewPlayer->PlayerController = PlayerController;

		
        auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
		
		FTransform LocationToSpawn;
        if (!bStartedBus)
            LocationToSpawn = GetPlayerStart(PlayerController);
        else
        {
            TArray<AFortPlayerPawn*> OutActors;
            GetFortKismet()->STATIC_GetAllFortPlayerPawns(GetWorld(), &OutActors);
			auto PlayerPawn = OutActors[0];
            LocationToSpawn.Translation = PlayerPawn->K2_GetActorLocation();
            LocationToSpawn.Translation.Z += 100;
            LocationToSpawn.Rotation = FQuat();
			LocationToSpawn.Scale3D = FVector{1, 1, 1};
        }
            

        InitInventory(PlayerController);

        auto Pawn = (APlayerPawn_Athena_C*)SpawnActorTrans(APlayerPawn_Athena_C::StaticClass(), LocationToSpawn, PlayerController);

        PlayerController->Pawn = Pawn;
        PlayerController->AcknowledgedPawn = Pawn;
        Pawn->Owner = PlayerController;
        Pawn->OnRep_Owner();
        PlayerController->OnRep_Pawn();
        PlayerController->Possess(Pawn);

        constexpr static auto Health = 100;
        const static auto Shield = 100;

        Pawn->SetMaxHealth(Globals::MaxHealth);
        Pawn->SetMaxShield(Globals::MaxShield);
        if (!bStartedBus)
            Pawn->bCanBeDamaged = false;
		
		Pawn->NetUpdateFrequency *= 2; // Original is 100.0f;
        auto CM = Pawn->CharacterMovement;

        if (CM)
        {
            switch (Mode)
            {
            case CustomMode::SPACE:
                CM->GravityScale -= 0.5f; // Default is 1.0f
                break;
            case CustomMode::JUGGERNAUT:
                // CM->Mass += 200; // Default is 100.0f // this does nothing for some reason
				// CM->MaxAcceleration -= 500.0f; // Default is 1000f // this does nothing for some reason
                break;
            }
        }
        else
            std::cout << "Could not find CharacterMovementComponent!\n";

        PlayerController->bHasClientFinishedLoading = true; // should we do this on ServerSetClientHasFinishedLoading 
        PlayerController->bHasServerFinishedLoading = true;
        PlayerController->bHasInitiallySpawned = true;
        PlayerController->OnRep_bHasServerFinishedLoading();

        Pawn->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Pawn->HealthRegenDelayGameplayEffect, Pawn->AbilitySystemComponent, 1);
        Pawn->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Pawn->HealthRegenGameplayEffect, Pawn->AbilitySystemComponent, 1);
        Pawn->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Pawn->ShieldRegenDelayGameplayEffect, Pawn->AbilitySystemComponent, 1);
        Pawn->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Pawn->ShieldRegenGameplayEffect, Pawn->AbilitySystemComponent, 1);
        // TODO: Check if this is useless.
        Pawn->HealthRegenDelayGameplayEffect = nullptr;
        Pawn->HealthRegenGameplayEffect = nullptr;
        Pawn->ShieldRegenDelayGameplayEffect = nullptr;
        Pawn->ShieldRegenGameplayEffect = nullptr;

        PlayerState->bHasFinishedLoading = true;
        PlayerState->bHasStartedPlaying = true;
        PlayerState->OnRep_bHasStartedPlaying();
        
        static auto FortRegisteredPlayerInfo = ((UFortGameInstance*)GetWorld()->OwningGameInstance)->RegisteredPlayers[0]; // UObject::FindObject<UFortRegisteredPlayerInfo>("FortRegisteredPlayerInfo Transient.FortEngine_0_1.FortGameInstance_0_1.FortRegisteredPlayerInfo_0_1");
        if (FortRegisteredPlayerInfo)
        {
            auto Hero = FortRegisteredPlayerInfo->AthenaMenuHeroDef;

            if (Hero)
            {
                PlayerState->HeroType = Hero->GetHeroTypeBP();
                PlayerState->OnRep_HeroType();

                for (auto i = 0; i < Hero->CharacterParts.Num(); i++)
                {
                    auto Part = Hero->CharacterParts[i];

                    if (!Part)
                        continue;
                    PlayerState->CharacterParts[i] = Part;
                }

                PlayerState->CharacterBodyType = Hero->CharacterParts[1]->BodyTypesPermitted;
                Pawn->CharacterBodyType = Hero->CharacterParts[1]->BodyTypesPermitted;
                Pawn->CharacterGender = Hero->CharacterParts[1]->GenderPermitted;
                PlayerState->OnRep_CharacterBodyType();
                PlayerState->OnRep_CharacterParts();
                PlayerState->OnRep_HeroType();
            }
        }

        static std::vector<UFortWeaponRangedItemDefinition*> doublePumpLoadout = {
            (UFortWeaponRangedItemDefinition*)Utils::GetRandomPickaxe() // Candy 
        };


        if (!PlayerController)
            std::cout << "Player Controller Is Not Valid." << std::endl;
             EquipLoadout(PlayerController, doublePumpLoadout);


      

        auto CheatManager = CreateCheatManager(PlayerController);
        CheatManager->ToggleInfiniteDurability();
        if (Globals::bRespawnPlayers)
            CheatManager->ToggleInfiniteAmmo();

        if (PlayerController->Pawn)
        {
            if (PlayerController->Pawn->PlayerState)
            {
                PlayerState->TeamIndex = EFortTeam(CurrentTeam);
                CurrentTeam++;
                PlayerState->OnRep_PlayerTeam();
                
            }
        }

        PlayerController->OverriddenBackpackSize = 100; // i hate stw

		// TODO: Remove healing GameplayEffects

        // Pawn->K2_TeleportTo({ 37713, -52942, 461 }, { 0, 0, 0 }); // Tilted

        

        return PlayerController;
    }

    void Beacon_NotifyControlMessage(AOnlineBeaconHost* Beacon, UNetConnection* Connection, uint8 MessageType, int64* Bunch)
    {
        printf("Recieved control message %i\n", MessageType);

        switch (MessageType)
        {
        case 4: // NMT_Netspeed
            Connection->CurrentNetSpeed = 30000;
            return;
        case 5: // NMT_Login
        {
            Bunch[7] += (16 * 1024 * 1024);

            auto OnlinePlatformName = FString(L"");

            Native::NetConnection::ReceiveFString(Bunch, Connection->ClientResponse);
            Native::NetConnection::ReceiveFString(Bunch, Connection->RequestURL);
            Native::NetConnection::ReceiveUniqueIdRepl(Bunch, Connection->PlayerID);
            Native::NetConnection::ReceiveFString(Bunch, OnlinePlatformName);

            Bunch[7] -= (16 * 1024 * 1024);

			
            Native::World::WelcomePlayer(GetWorld(), Connection);
            return;
        }
        case 15: // NMT_PCSwap
            // return;
            break;
        }

        Native::World::NotifyControlMessage(GetWorld(), Connection, MessageType, Bunch);
    }

    uint8 Beacon_NotifyAcceptingConnection(AOnlineBeacon* Beacon)
    {
        return Native::World::NotifyAcceptingConnection(GetWorld());
    }

    void* SeamlessTravelHandlerForWorld(UEngine* Engine, UWorld* World)
    {
        return Native::Engine::SeamlessTravelHandlerForWorld(Engine, GetWorld());
    }

    void* NetDebug(UObject* _this)
    {
        return nullptr;
    }

    void PostRender(UGameViewportClient* _this, UCanvas* Canvas)
    {
        ZeroGUI::SetupCanvas(Canvas);
        GUI::Tick();

        return Native::GameViewportClient::PostRender(_this, Canvas);
    }

    __int64 CollectGarbage(__int64 a1)
    {
        return 0;
    };

    void InitNetworkHooks()
    {
        DETOUR_START
        DetourAttachE(Native::World::WelcomePlayer, WelcomePlayer);
        DetourAttachE(Native::Actor::GetNetMode, GetNetMode);
        DetourAttachE(Native::World::NotifyControlMessage, World_NotifyControlMessage);
        DetourAttachE(Native::World::SpawnPlayActor, SpawnPlayActor);
        DetourAttachE(Native::OnlineBeaconHost::NotifyControlMessage, Beacon_NotifyControlMessage);
        DetourAttachE(Native::OnlineSession::KickPlayer, KickPlayer);
        DetourAttachE(Native::GameViewportClient::PostRender, PostRender);
        DetourAttachE(Native::GC::CollectGarbage, CollectGarbage);
        DetourAttachE(Native::OnReload, OnReloadHook);
        DETOUR_END
    }
	
    void DetachNetworkHooks()
    {
        DETOUR_START
        DetourDetachE(Native::World::WelcomePlayer, WelcomePlayer);
        DetourDetachE(Native::Actor::GetNetMode, GetNetMode);
        DetourDetachE(Native::World::NotifyControlMessage, World_NotifyControlMessage);
        DetourDetachE(Native::World::SpawnPlayActor, SpawnPlayActor);
        DetourDetachE(Native::OnlineBeaconHost::NotifyControlMessage, Beacon_NotifyControlMessage);
        DetourDetachE(Native::OnlineSession::KickPlayer, KickPlayer);
        DetourDetachE(Native::GameViewportClient::PostRender, PostRender);
        DetourDetachE(Native::GC::CollectGarbage, CollectGarbage);
        DetourDetachE(Native::OnReload, OnReloadHook)
        DETOUR_END
    }

	

    static AFortPickupAthena* SpawnPickup(FVector Location, UFortItemDefinition* ItemDef = Utils::GetRandomItemDefinition(), int Count = 1)
    {
        if (Count == 1 && ItemDef->IsA(UFortAmmoItemDefinition::StaticClass()))
            Count = ((UFortAmmoItemDefinition*)(ItemDef))->DropCount;
		
        FTransform Transform;
        Transform.Scale3D = FVector(1, 1, 1);
        Transform.Rotation = FQuat();
        Transform.Translation = Location; // Next to salty
		
        auto Pickup = (AFortPickupAthena*)SpawnActorTrans(AFortPickupAthena::StaticClass(), Transform, nullptr);

        Pickup->PrimaryPickupItemEntry.ItemDefinition = ItemDef;
        Pickup->PrimaryPickupItemEntry.Count = Count;
        
        

        Pickup->TossPickup(Location, nullptr, 6, true);

        return Pickup;
    }

    

    void ProcessEventHook(UObject* Object, UFunction* Function, void* Parameters)
    {
		auto FuncName = Function->GetName();
        if (FuncName == "ServerAttemptInteract")
        {
            auto PlayerController = (AFortPlayerControllerAthena*)Object;
            auto CurrentParams = (AFortPlayerController_ServerAttemptInteract_Params*)Parameters;

            auto ReceivingActor = CurrentParams->ReceivingActor;

            if (ReceivingActor && ReceivingActor->Class->GetName().contains("Tiered_Short_Ammo"))
            {
                auto Ammo = (ABuildingContainer*)ReceivingActor;
                Ammo->bAlreadySearched = true;
                Ammo->OnRep_bAlreadySearched();

                auto Location = ReceivingActor->K2_GetActorLocation();
                auto AmmoSound = Utils::FindObjectFast<USoundCue>("/Game/Sounds/Foley_Loot/Containers/Toolbox/Toolbox_SearchEnd_Cue.Toolbox_SearchEnd_Cue");
                PlayerController->ClientPlaySoundAtLocation(AmmoSound, Location, 1, 1);
                for (int i = 0; i < 2; i++)
                {
                    SpawnPickup(Location, Utils::GetRandomAmmoItemDefinition());
                }
            }
			
			/* if (ReceivingActor && ReceivingActor->Class->GetName().contains("VendingMachine"))
            {
              auto VendingMachine = (ABuildingItemCollectorActor*)ReceivingActor;
              auto Location = ReceivingActor->K2_GetActorLocation();
              VendingMachine->LootSpawnLocation = Location;
              VendingMachine->LootSpawnLocation.X = VendingMachine->GetActorForwardVector().X + 10;
              VendingMachine->ItemCollections[0].OutputItem = (UFortWorldItemDefinition*)Utils::GetRandomItemDefinition();
              if (!VendingMachine->ItemCollections[0].OutputItem)
                  std::cout << "No input item" << std::endl;
              auto Pickup = SpawnPickup(VendingMachine->LootSpawnLocation, VendingMachine->ItemCollections[0].OutputItem);
              
            } */

            if (ReceivingActor && ReceivingActor->Class->GetName().contains("Tiered_Chest"))
            {
                auto Chest = (ABuildingContainer*)ReceivingActor;
                std::cout << "auto Chest = (ABuildingContainer*)ReceivingActor" << std::endl;
                Chest->bAlreadySearched = true;
                std::cout << "Chest->bAlreadySearched = true;" << std::endl;
                Chest->OnRep_bAlreadySearched();
                std::cout << "Chest->OnRep_bAlreadySearched();" << std::endl;

                auto Location = ReceivingActor->K2_GetActorLocation();
                std::cout << "Location Set Successfully" << std::endl;

				UFortWeaponItemDefinition* WeaponDef;
                if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.39f))
                {
                    WeaponDef = Utils::GetRandomUncommonWeaponDefinition();
                }
                else if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.35f))
                    WeaponDef = Utils::GetRandomRareWeaponDefinition();
                else if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.22f))
                    WeaponDef = Utils::GetRandomEpicWeaponDefinition();
                else if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.15f))
                    WeaponDef = Utils::GetRandomGoldWeaponDefinition();
                else
                    WeaponDef = Utils::GetRandomCommonWeaponDefinition();
                if (WeaponDef)
                {
                    std::cout << "Weapon Def Set Successfully!" << std::endl;
                }
                else
                    WeaponDef = (UFortWeaponItemDefinition*)Utils::GetRandomItemDefinition();
                auto ChestSound = Utils::FindObjectFast<USoundCue>("/Game/Sounds/Foley_Loot/Containers/Treasure_Chest/Tiered_Chest_Open_T01_Cue.Tiered_Chest_Open_T01_Cue");
                PlayerController->ClientPlaySoundAtLocation(ChestSound, Location, 1, 1);
                auto Pickup = Hooks::SpawnPickup(Location, WeaponDef);
                if (WeaponDef->IsA(UFortWeaponItemDefinition::StaticClass()))
                {
                    auto WeaponWid = (UFortWeaponItemDefinition*)WeaponDef;
                    auto ActorClass = (AFortWeapon*)WeaponWid->GetWeaponActorClass();

                }
                if (Pickup)
                {
                    std::cout << "Pickup Spawned and Valid!" << std::endl;
                }
                if (Globals::bSpawnConsumables)
              SpawnPickup(Location, Utils::GetRandomConsumableItemDefinition());
                std::cout << "Spawned Consumable!" << std::endl;
               SpawnPickup(Location, ((UFortWeaponItemDefinition*)WeaponDef)->GetAmmoWorldItemDefinition_BP(), WeaponDef->GetAmmoWorldItemDefinition_BP()->DropCount);
                std::cout << "Spawned Ammo!" << std::endl;
               SpawnPickup(Location, Utils::GetRandomResourceItemDefinition(), 30);
                std::cout << "Spawned Resource!" << std::endl;
               if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.25) && Globals::bSpawnTraps)
                   SpawnPickup(Location, Utils::GetRandomTrap(), 3);
            }

            if (ReceivingActor && ReceivingActor->Class->GetName().contains("AthenaSupplyDrop")) // AllyJ - WIP Supply Drop Loot
            {
                auto AthenaSupplyDrop = (ABuildingContainer*)ReceivingActor;
                auto Location = ReceivingActor->K2_GetActorLocation();

                auto WeaponDef = Utils::GetRandomGoldWeaponDefinition();
                auto Pickup = Hooks::SpawnPickup(Location, WeaponDef);
                SpawnPickup(Location, Utils::GetRandomTrap(), 1);
                
				
                if (Globals::bSpawnConsumables)
                {
                    SpawnPickup(Location, Utils::GetRandomConsumableItemDefinition());
                    SpawnPickup(Location, Utils::GetRandomConsumableItemDefinition());
                }
                SpawnPickup(Location, ((UFortWeaponItemDefinition*)WeaponDef)->GetAmmoWorldItemDefinition_BP());
                SpawnPickup(Location, Utils::GetRandomResourceItemDefinition(), 30);
                SpawnPickup(Location, Utils::GetRandomResourceItemDefinition(), 30);
                SpawnPickup(Location, Utils::GetRandomResourceItemDefinition(), 30);
                AthenaSupplyDrop->K2_DestroyActor();
                
            }
        }

        if (Function->GetFullName() == "Function FortniteGame.FortPlayerController.ClientReportDamagedResourceBuilding")
        {
            auto Controller = (AFortPlayerControllerAthena*)Object;
            auto Params = (AFortPlayerController_ClientReportDamagedResourceBuilding_Params*)Parameters;
            auto ResourceType = Params->PotentialResourceType.GetValue();
            UFortResourceItemDefinition* WorldItemDefinition = nullptr;
            static auto WoodDefinition = Utils::FindObjectFast<UFortResourceItemDefinition>(Utils::ResourcePool[0]);
            static auto StoneDefinition = Utils::FindObjectFast<UFortResourceItemDefinition>(Utils::ResourcePool[1]);
            static auto MetalDefinition = Utils::FindObjectFast<UFortResourceItemDefinition>(Utils::ResourcePool[2]);

            switch (ResourceType)
            {
            case EFortResourceType::Wood:
                WorldItemDefinition = WoodDefinition;
                break;
            case EFortResourceType::Stone:
                WorldItemDefinition = StoneDefinition;
                break;
            case EFortResourceType::Metal:
                WorldItemDefinition = MetalDefinition;
                break;
            }

            bool bFound = false;

            if (WorldItemDefinition)
            {
                for (int i = 0; i < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
                {
                    if (Controller->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition == WorldItemDefinition)
                    {
                        bFound = true;
                        Controller->WorldInventory->Inventory.ReplicatedEntries[i].Count += Params->PotentialResourceCount;
                        Controller->WorldInventory->Inventory.ReplicatedEntries[i].ReplicationKey++;

                        if (Controller->WorldInventory->Inventory.ReplicatedEntries[i].Count <= 0)
                        {
                            Controller->WorldInventory->Inventory.ReplicatedEntries.RemoveSingle(i);

                            for (int j = 0; j < Controller->WorldInventory->Inventory.ItemInstances.Num(); j++)
                            {
                                auto ItemInstance = Controller->WorldInventory->Inventory.ItemInstances[j];

                                if (ItemInstance && ItemInstance->GetItemDefinitionBP() == WorldItemDefinition)
                                {
                                    Controller->WorldInventory->Inventory.ItemInstances.RemoveSingle(i);
                                    break;
                                }
                                else
                                    continue;
                            }
                        }
                        else if (Controller->WorldInventory->Inventory.ReplicatedEntries[i].Count > 999)
                        {
                            Hooks::SpawnPickup(Controller->Pawn->K2_GetActorLocation(), WorldItemDefinition, Controller->WorldInventory->Inventory.ReplicatedEntries[i].Count - 999);
							Controller->WorldInventory->Inventory.ReplicatedEntries[i].Count = 999;
                            Inventory::Update(Controller, 0, true);
                        }

                        Inventory::Update(Controller, 0, true);
                        break;
                    }
                    else
                        continue;
                }

                if (!bFound)
                {
                    for (int i = 0; i < Controller->QuickBars->SecondaryQuickBar.Slots.Num(); i++)
                    {
                        if (!Controller->QuickBars->SecondaryQuickBar.Slots[i].Items.Data) // Checks if the slot is empty
                        {
                            Inventory::AddItemToSlot(Controller, WorldItemDefinition, i, EFortQuickBars::Secondary, Params->PotentialResourceCount);
                            break;
                        }
                    }
                }
            }
        }

        // ToDo: Proper material gather amount and weak spot multiplier
        if (Function->GetName() == "OnDamagePlayEffects" || Function->GetName() == "OnDeathPlayEffects")
        {
            if (!Object->IsA(AFortPlayerPawnAthena::StaticClass()))
            {
                auto Params = (ABuildingActor_OnDamagePlayEffects_Params*)Parameters;
                if (Params->InstigatedBy)
                {
                    auto Controller = (AFortPlayerControllerAthena*)Params->InstigatedBy->Controller;
                    // if (Controller->WeakspotUnderReticle.IsValid())
                    // LOG_INFO("WeakSpot Valid");

                    if (Controller && Params->DamageCauser->GetFullName().find("Melee") != -1)
                    {
                        auto Obj = (ABuildingSMActor*)Object;
                        if (Obj->bPlayerPlaced != true)
                            Controller->ClientReportDamagedResourceBuilding(Obj, Obj->ResourceType, GetMath()->STATIC_RandomFloatInRange(5, 11), Function->GetName() == "OnDeathPlayEffects" ? true : false, false);
                    }
                }
            }
        }

        if (Function->GetFullName() == "Function SafeZoneIndicator.SafeZoneIndicator_C.OnSafeZoneStateChange" && bLateGame)
        {
            auto Indicator = (ASafeZoneIndicator_C*)Object;
            auto SafeZonePhase = ((AFortGameModeAthena*)GetWorld()->AuthorityGameMode)->SafeZonePhase;
            auto GameState = (AFortGameStateAthena*)GetWorld()->GameState;
            Indicator->NextCenter = (FVector_NetQuantize100)BusLocation;

            switch (SafeZonePhase)
            {
            case 0:

                Indicator->Radius = 15000;
                Indicator->NextRadius = 11000;
                break;
            case 1:
                Indicator->NextRadius = 7000;
                break;
            case 2:
                Indicator->NextRadius = 4000;
                break;
            case 3:
                Indicator->NextRadius = 1000;
                break;
            case 4:
                Indicator->NextRadius = 500;
                break;
            default:
                Indicator->NextRadius = 50;
                break;
            }
        }	

        if (Function->GetName().find("Tick") != std::string::npos && bRestart)
        {
            bRestart = false;
            bTraveled = false;
            bPlayButton = false;
            bListening = false;
            bSpawnedFloorLoot = false;
            bStartedBus = false;
            PlayersJumpedFromBus = 0;
            HostBeacon = nullptr;
            ((AFortGameModeAthena*)GetWorld()->AuthorityGameMode)->bSafeZonePaused = false;
            DetachNetworkHooks();
            ExistingBuildings.FreeArray();
            GetKismetSystem()->STATIC_ExecuteConsoleCommand(GetWorld(), L"open frontend", GetPlayerController());
        }
        
       

        if (!bPlayButton)
        {
            static auto PlayButtonFn = UObject::FindObject<UFunction>("BndEvt__BP_PlayButton_K2Node_ComponentBoundEvent_1_CommonButtonClicked__DelegateSignature");

            // if (FunctionName.find("BP_PlayButton") != -1)
            if (Function == PlayButtonFn)
            {
                bPlayButton = true;
                Game::Start();
                printf("[Game::Start] Done\n");

                InitNetworkHooks();
                printf("[InitNetworkHooks] Done\n");
            }
            
        }

        if (bTraveled)
        {
#ifdef LOGGING
            auto FunctionName = Function->GetName();
            if (Function->FunctionFlags & 0x00200000 || (Function->FunctionFlags & 0x01000000 && FunctionName.find("Ack") == -1 && FunctionName.find("AdjustPos") == -1))
            {
                if (FunctionName.find("ServerUpdateCamera") == -1 && FunctionName.find("ServerMove") == -1)
                {
                    std::cout << "RPC Called: " << FunctionName << '\n';
                }
            }
#endif

            for (int i = 0; i < UFunctionHooks::toHook.size(); i++)
            {
                if (Function == UFunctionHooks::toHook[i])
                {
                    if (UFunctionHooks::toCall[i](Object, Parameters))
                    {
                        return;
                    }
                    break;
                }
            }
        }

        return ProcessEvent(Object, Function, Parameters);
    }
}
