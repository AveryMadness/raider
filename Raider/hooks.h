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

int CurrentTeam = 2;

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

    APlayerController* SpawnPlayActor(UWorld* World, UPlayer* NewPlayer, ENetRole RemoteRole, FURL& URL, void* UniqueId, SDK::FString& Error, uint8 NetPlayerIndex)
    {
        auto PlayerController = (AFortPlayerControllerAthena*)Native::World::SpawnPlayActor(GetWorld(), NewPlayer, RemoteRole, URL, UniqueId, Error, NetPlayerIndex);
        NewPlayer->PlayerController = PlayerController;

        auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
		
		
            

        InitInventory(PlayerController);

        auto Pawn = (APlayerPawn_Athena_C*)SpawnActorTrans(APlayerPawn_Athena_C::StaticClass(), GetPlayerStart(PlayerController), PlayerController);

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
        Pawn->HealthSet->CurrentShield.Minimum = 0.0f;
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
            }
        }

        static std::vector<UFortWeaponRangedItemDefinition*> doublePumpLoadout = {
            FindWID("WID_Harvest_Pickaxe_Athena_C_T01") // Candy 
        };


        if (!PlayerController)
            std::cout << "Player Controller Is Not Valid." << std::endl;
             EquipLoadout(PlayerController, doublePumpLoadout);


      

        auto CheatManager = CreateCheatManager(PlayerController);
        CheatManager->ToggleInfiniteAmmo();
        CheatManager->ToggleInfiniteDurability();

        if (PlayerController->Pawn)
        {
            if (PlayerController->Pawn->PlayerState)
            {
                PlayerState->TeamIndex = EFortTeam(CurrentTeam); 
                std::cout << "Joining Team:" << CurrentTeam << std::endl;
                CurrentTeam++;
                std::cout << "Incrementing Current Team! New Team Number: " << CurrentTeam << std::endl;
                PlayerState->OnRep_PlayerTeam();
                PlayerState->SquadId = PlayerState->PlayerTeam->TeamMembers.Num() + 1;
                PlayerState->OnRep_SquadId();
                
            }
        }

        PlayerController->OverriddenBackpackSize = 100; // i hate stw

		// TODO: Remove healing GameplayEffects

        // Pawn->K2_TeleportTo({ 37713, -52942, 461 }, { 0, 0, 0 }); // Tilted

        auto SpawnLoc = Pawn->K2_GetActorLocation();
        FTransform SpawnTransform;
        SpawnTransform.Rotation.X = Pawn->K2_GetActorRotation().Pitch;
        SpawnTransform.Rotation.Y = Pawn->K2_GetActorRotation().Yaw;
        SpawnTransform.Rotation.Z = Pawn->K2_GetActorRotation().Roll;
        SpawnTransform.Translation = SpawnLoc;
        SpawnTransform.Scale3D = FVector(1, 1, 1);
        auto Drone = (ABP_VictoryDrone_C*)SpawnActorTrans(ABP_VictoryDrone_C::StaticClass(), SpawnTransform, PlayerController);
        Drone->InitDrone();
        Drone->TriggerPlayerSpawnEffects();

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

    bool GuidComp(FGuid guidA, FGuid guidB)
    {
        if (guidA.A == guidB.A && guidA.B == guidB.B && guidA.C == guidB.C && guidA.D == guidB.D)
            return true;
        else
            return false;
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
                if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.59f))
                {
                    WeaponDef = Utils::GetRandomUncommonWeaponDefinition();
                }
                else if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.35f))
                    WeaponDef = Utils::GetRandomRareWeaponDefinition();
                else if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.12f))
                    WeaponDef = Utils::GetRandomEpicWeaponDefinition();
                else if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.5f))
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

                    Pickup->PrimaryPickupItemEntry.LoadedAmmo = ActorClass->GetBulletsPerClip();
                }
                if (Pickup)
                {
                    std::cout << "Pickup Spawned and Valid!" << std::endl;
                }

               SpawnPickup(Location, Utils::GetRandomConsumableItemDefinition());
                std::cout << "Spawned Consumable!" << std::endl;
               SpawnPickup(Location, ((UFortWeaponItemDefinition*)WeaponDef)->GetAmmoWorldItemDefinition_BP(), 29);
                std::cout << "Spawned Ammo!" << std::endl;
               SpawnPickup(Location, Utils::GetRandomResourceItemDefinition(), 30);
                std::cout << "Spawned Resource!" << std::endl;
            }

            if (ReceivingActor && ReceivingActor->Class->GetName().contains("AthenaSupplyDrop")) // AllyJ - WIP Supply Drop Loot
            {
                auto AthenaSupplyDrop = (ABuildingContainer*)ReceivingActor;
                auto Location = ReceivingActor->K2_GetActorLocation();

                auto WeaponDef = Utils::GetRandomGoldWeaponDefinition();
                auto Pickup = Hooks::SpawnPickup(Location, WeaponDef);

                SpawnPickup(Location, Utils::GetRandomConsumableItemDefinition());
                SpawnPickup(Location, Utils::GetRandomConsumableItemDefinition());
                SpawnPickup(Location, ((UFortWeaponItemDefinition*)WeaponDef)->GetAmmoWorldItemDefinition_BP());
                SpawnPickup(Location, Utils::GetRandomResourceItemDefinition(), 30);
                SpawnPickup(Location, Utils::GetRandomResourceItemDefinition(), 30);
                SpawnPickup(Location, Utils::GetRandomResourceItemDefinition(), 30);
                AthenaSupplyDrop->K2_DestroyActor();
                
            }
        }

        if (FuncName == "ServerHandlePickup")
        {
            
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
