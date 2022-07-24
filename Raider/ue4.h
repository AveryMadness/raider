#pragma once

#include <unordered_set>
#include <random>

#include "native.h"


inline bool bTraveled = false;
inline bool bPlayButton = false;
inline bool bListening = false;
static bool bSpawnedFloorLoot = false;
static bool bRestart = false;
static FVector BusLocation;
int PlayersJumpedFromBus = 0;

static std::unordered_set<ABuildingSMActor*> Buildings;
static TArray<ABuildingActor*> ExistingBuildings;
static AFortOnlineBeaconHost* HostBeacon = nullptr;

inline UWorld* GetWorld()
{
    return GetEngine()->GameViewport->World;
    // return *(UWorld**)(Offsets::Imagebase + Offsets::GWorldOffset);
}



inline AAthena_PlayerController_C* GetPlayerController(int32 Index = 0)
{
    if (Index > GetWorld()->OwningGameInstance->LocalPlayers.Num())
    {
        std::cout << "WARNING! PlayerController out of range! (" << Index << " out of " << GetWorld()->OwningGameInstance->LocalPlayers.Num() << ")" << '\n';

        return (AAthena_PlayerController_C*)GetWorld()->OwningGameInstance->LocalPlayers[0]->PlayerController;
    }

    return (AAthena_PlayerController_C*)GetWorld()->OwningGameInstance->LocalPlayers[Index]->PlayerController;
}

struct FNetworkObjectInfo
{
    AActor* Actor;

    TWeakObjectPtr<AActor> WeakActor;

    double NextUpdateTime;

    double LastNetReplicateTime;

    float OptimalNetUpdateDelta;

    float LastNetUpdateTime;

    uint32 bPendingNetUpdate : 1;

    uint32 bForceRelevantNextUpdate : 1;

    TSet<TWeakObjectPtr<UNetConnection>> DormantConnections;

    TSet<TWeakObjectPtr<UNetConnection>> RecentlyDormantConnections;

    bool operator==(const FNetworkObjectInfo& Other)
    {
        return Actor == Other.Actor;
    }
};

class FNetworkObjectList
{
public:
    using FNetworkObjectSet = TSet<TSharedPtr<FNetworkObjectInfo>>;

    FNetworkObjectSet AllNetworkObjects;
    FNetworkObjectSet ActiveNetworkObjects;
    FNetworkObjectSet ObjectsDormantOnAllConnections;

    TMap<TWeakObjectPtr<UObject>, int32> NumDormantObjectsPerConnection;
};

FORCEINLINE int32& GetReplicationFrame(UNetDriver* Driver)
{
    return *(int32*)(int64(Driver) + 816); // Offsets::Net::ReplicationFrame);
}

FORCEINLINE auto& GetNetworkObjectList(UObject* NetDriver)
{
    return *(*(TSharedPtr<FNetworkObjectList>*)(int64(NetDriver) + 0x508));
}

FORCEINLINE UGameplayStatics* GetGameplayStatics()
{
    return reinterpret_cast<UGameplayStatics*>(UGameplayStatics::StaticClass());
}

FORCEINLINE UKismetSystemLibrary* GetKismetSystem()
{
    return reinterpret_cast<UKismetSystemLibrary*>(UKismetSystemLibrary::StaticClass());
}

FORCEINLINE UFortKismetLibrary* GetFortKismet()
{
    return ((UFortKismetLibrary*)UFortKismetLibrary::StaticClass());
}

FORCEINLINE UKismetStringLibrary* GetKismetString()
{
    return (UKismetStringLibrary*)UKismetStringLibrary::StaticClass();
}

static FVector GetRandomBattleBusLocation()
{
    static std::vector<FVector> Locations = {
        { 24426, 37710, 17525 }, // Retail Row
        { 50018, 73844, 17525 }, // Lonely Lodge
        { 34278, 867, 9500 }, // Dusty Depot & The Factories
        { 79710, 15677, 17525 }, // Tomato Town
        { 103901, -20203, 17525 }, // Anarchy Acres
        { 86766, -83071, 17525 }, // Pleasant Park
        { 2399, -96255, 17525 }, // Greasy Grove
        { -35037, -463, 13242 }, // Fatal Fields
        { 83375, 50856, 17525 }, // Wailing Woods
        { 35000, -60121, 20525 }, // Tilted Towers
        { 40000, -127121, 17525 }, // Snobby Shores
        { 5000, -60121, 10748 }, // Shifty Shafts
        { 110088, -115332, 17525 }, // Haunted Hills
        { 119126, -86354, 17525 }, // Junk Houses
        { 130036, -105092, 17525 }, // Junk Junction
        { 39781, 61621, 17525 }, // Moisty Mire
        { -68000, -63521, 17525 }, // Flush Factory
        { 3502, -9183, 10500 }, // Salty Springs
        { 7760, 76702, 17525 }, // Race Track
        { 38374, -94726, 17525 }, // Soccer field
        { 70000, -40121, 17525 }, // Loot Lake
        { -123778, -112480, 17525 } // Spawn Island
    };

    static auto Location = Locations[rand() % Locations.size()];
    return Location;
}

static FORCEINLINE void sinCos(float* ScalarSin, float* ScalarCos, float Value)
{
    float quotient = (INV_PI * 0.5f) * Value;
    if (Value >= 0.0f)
    {
        quotient = (float)((int)(quotient + 0.5f));
    }
    else
    {
        quotient = (float)((int)(quotient - 0.5f));
    }
    float y = Value - (2.0f * PI) * quotient;

    float sign;
    if (y > HALF_PI)
    {
        y = PI - y;
        sign = -1.0f;
    }
    else if (y < -HALF_PI)
    {
        y = -PI - y;
        sign = -1.0f;
    }
    else
    {
        sign = +1.0f;
    }

    float y2 = y * y;

    *ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

    float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
    *ScalarCos = sign * p;
}

static auto RotToQuat(FRotator Rotator)
{
    const float DEG_TO_RAD = PI / (180.f);
    const float DIVIDE_BY_2 = DEG_TO_RAD / 2.f;
    float SP, SY, SR;
    float CP, CY, CR;

    sinCos(&SP, &CP, Rotator.Pitch * DIVIDE_BY_2);
    sinCos(&SY, &CY, Rotator.Yaw * DIVIDE_BY_2);
    sinCos(&SR, &CR, Rotator.Roll * DIVIDE_BY_2);

    FQuat RotationQuat;
    RotationQuat.X = CR * SP * SY - SR * CP * CY;
    RotationQuat.Y = -CR * SP * CY - SR * CP * SY;
    RotationQuat.Z = CR * CP * SY - SR * SP * CY;
    RotationQuat.W = CR * CP * CY + SR * SP * SY;

    return RotationQuat;
}

static auto VecToRot(FVector Vector)
{
    FRotator R;

    R.Yaw = std::atan2(Vector.Y, Vector.X) * (180.f / PI);

    R.Pitch = std::atan2(Vector.Z, std::sqrt(Vector.X * Vector.X + Vector.Y * Vector.Y)) * (180.f / PI);

    // roll can't be found from vector
    R.Roll = 0;

    return R;
}

static AActor* SpawnActorTrans(UClass* StaticClass, FTransform SpawnTransform, AActor* Owner = nullptr, ESpawnActorCollisionHandlingMethod Flags = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)
{
    AActor* FirstActor = GetGameplayStatics()->STATIC_BeginDeferredActorSpawnFromClass(GetWorld(), StaticClass, SpawnTransform, Flags, Owner);

    if (FirstActor)
    {
        AActor* FinalActor = GetGameplayStatics()->STATIC_FinishSpawningActor(FirstActor, SpawnTransform);

        if (FinalActor)
        {
            return FinalActor;
        }
    }

    return nullptr;
}

inline auto& GetItemInstances(AFortPlayerController* PC)
{
    return PC->WorldInventory->Inventory.ItemInstances;
}

inline AActor* SpawnActor(UClass* ActorClass, FVector Location = { 0.0f, 0.0f, 0.0f }, FRotator Rotation = { 0, 0, 0 }, AActor* Owner = nullptr, ESpawnActorCollisionHandlingMethod Flags = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)
{
    FTransform SpawnTransform;

    SpawnTransform.Translation = Location;
    SpawnTransform.Scale3D = FVector{ 1, 1, 1 };
    SpawnTransform.Rotation = RotToQuat(Rotation);

    return SpawnActorTrans(ActorClass, SpawnTransform, Owner, Flags);
}

template <typename RetActorType = AActor>
inline RetActorType* SpawnActor(FVector Location = { 0.0f, 0.0f, 0.0f }, AActor* Owner = nullptr, FQuat Rotation = { 0, 0, 0 }, ESpawnActorCollisionHandlingMethod Flags = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)
{
    FTransform SpawnTransform;

    SpawnTransform.Translation = Location;
    SpawnTransform.Scale3D = FVector{ 1, 1, 1 };
    SpawnTransform.Rotation = Rotation;

    return (RetActorType*)SpawnActorTrans(RetActorType::StaticClass(), SpawnTransform, Owner, Flags);
}

inline ABuildingSMActor* SpawnBuilding(UClass* BGAClass, FVector& Location, FRotator& Rotation, APlayerPawn_Athena_C* Pawn)
{
    FTransform Transform;
    Transform.Translation = Location;
    Transform.Scale3D = FVector{ 1, 1, 1 };
    Transform.Rotation = RotToQuat(Rotation);

    return (ABuildingSMActor*)GetFortKismet()->STATIC_SpawnBuildingGameplayActor(BGAClass, Transform, Pawn);
}

inline void CreateConsole()
{
    GetEngine()->GameViewport->ViewportConsole = (UConsole*)GetGameplayStatics()->STATIC_SpawnObject(UConsole::StaticClass(), GetEngine()->GameViewport);
}

inline auto CreateCheatManager(APlayerController* Controller)
{
    if (!Controller->CheatManager)
    {
        Controller->CheatManager = (UCheatManager*)GetGameplayStatics()->STATIC_SpawnObject(UFortCheatManager::StaticClass(), Controller); // lets just assume its gamemode athena
    }

    return (UFortCheatManager*)Controller->CheatManager;
}



DWORD WINAPI SleepForGameEnd(LPVOID)
{
    Sleep(30000);
    bRestart = true;
}

bool CanBuild(ABuildingSMActor* BuildingActor)
{
    bool bCanBuild = true;

    for (int i = 0; i < ExistingBuildings.Num(); i++)
    {
        auto Building = ExistingBuildings[i];

        if (!Building)
            continue;

        if (Building->K2_GetActorLocation() == BuildingActor->K2_GetActorLocation() && Building->BuildingType == BuildingActor->BuildingType)
        {
            bCanBuild = false;
        }
    }

    if (bCanBuild || ExistingBuildings.Num() == 0)
    {
        ExistingBuildings.Add(BuildingActor);

        return true;
    }

    return false;
}

bool CanBuild2(ABuildingSMActor* BuildingActor)
{
    static auto GameState = reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState);

    TArray<ABuildingActor*> ExistingBuildings;
    EFortStructuralGridQueryResults bCanBuild;
    bCanBuild = GameState->StructuralSupportSystem->K2_CanAddBuildingActorToGrid(GetWorld(), BuildingActor, BuildingActor->K2_GetActorLocation(), BuildingActor->K2_GetActorRotation(), false, false, &ExistingBuildings);

	if (bCanBuild == EFortStructuralGridQueryResults::CanAdd || ExistingBuildings.Num() == 0)
        return true;
	
    return false;
}

bool IsCurrentlyDisconnecting(UNetConnection* Connection)
{
    return false;
}

void Spectate(UNetConnection* SpectatingConnection, AFortPlayerStateAthena* StateToSpectate)
{
    if (!SpectatingConnection || !StateToSpectate)
        return;

    auto PawnToSpectate = StateToSpectate->GetCurrentPawn();
    auto DeadPC = static_cast<AFortPlayerControllerAthena*>(SpectatingConnection->PlayerController);

    if (!DeadPC)
        return;

    auto DeadPlayerState = static_cast<AFortPlayerStateAthena*>(DeadPC->PlayerState);

    if (!IsCurrentlyDisconnecting(SpectatingConnection) && DeadPlayerState && PawnToSpectate)
    {
        DeadPC->PlayerToSpectateOnDeath = PawnToSpectate;
        DeadPC->ClientSetSpectatorCamera(PawnToSpectate->K2_GetActorLocation(), PawnToSpectate->K2_GetActorRotation());
        DeadPC->SpectateOnDeath();

        DeadPlayerState->SpectatingTarget = StateToSpectate;
        DeadPlayerState->bIsSpectator = true;
        DeadPlayerState->OnRep_SpectatingTarget();

        // 95% of the code below here is useless, it was my attempt to fix the camera.

        if (DeadPC->QuickBars)
            DeadPC->QuickBars->K2_DestroyActor();
    }
}

inline void UpdateInventory(AFortPlayerController* PlayerController, int Dirty = 0, bool bRemovedItem = false)
{
    PlayerController->WorldInventory->bRequiresLocalUpdate = true;
    PlayerController->WorldInventory->HandleInventoryLocalUpdate();
    PlayerController->HandleWorldInventoryLocalUpdate();
    PlayerController->ForceUpdateQuickbar(EFortQuickBars::Primary);
    PlayerController->QuickBars->OnRep_PrimaryQuickBar();
    PlayerController->QuickBars->OnRep_SecondaryQuickBar();
    PlayerController->QuickBars->ForceNetUpdate();

    if (bRemovedItem)
        PlayerController->WorldInventory->Inventory.MarkArrayDirty();
}

inline auto AddItem(AFortPlayerController* PC, UFortItemDefinition* Def, int Slot, EFortQuickBars Bars = EFortQuickBars::Primary, int Count = 1)
{
    if (!PC || !Def)
        return FFortItemEntry();

    if (Def->IsA(UFortWeaponItemDefinition::StaticClass()))
        Count = 1;

    if (Slot < 0)
        Slot = 1;

    if (Bars == EFortQuickBars::Primary && Slot >= 6)
        Slot = 5;

    auto& QuickBarSlots = PC->QuickBars->PrimaryQuickBar.Slots;

    auto TempItemInstance = (UFortWorldItem*)Def->CreateTemporaryItemInstanceBP(Count, 1);

    if (TempItemInstance)
    {
        TempItemInstance->SetOwningControllerForTemporaryItem(PC);

        TempItemInstance->ItemEntry.Count = Count;
        TempItemInstance->OwnerInventory = PC->WorldInventory;

        auto& ItemEntry = TempItemInstance->ItemEntry;

        auto Idx = PC->WorldInventory->Inventory.ReplicatedEntries.Add(ItemEntry);

        GetItemInstances(PC).Add((UFortWorldItem*)TempItemInstance);
        PC->QuickBars->ServerAddItemInternal(ItemEntry.ItemGuid, Bars, Slot);

        if (Idx && PC->WorldInventory->Inventory.ReplicatedEntries.Num() >= Idx)
            PC->WorldInventory->Inventory.MarkItemDirty(PC->WorldInventory->Inventory.ReplicatedEntries[Idx]);

        return ItemEntry;
    }

    return FFortItemEntry();
}

inline auto AddItemWithUpdate(AFortPlayerController* PC, UFortItemDefinition* Def, int Slot, EFortQuickBars Bars = EFortQuickBars::Primary, int Count = 1)
{
    auto ItemEntry = AddItem(PC, Def, Slot, Bars, Count);

    UpdateInventory(PC);

    return ItemEntry;
}

void SetPartsFromCID(AFortPlayerControllerAthena* PC, UAthenaCharacterItemDefinition* CID)
{
	if (!PC || !CID)
		return;
}

inline UFortItemDefinition* GetDefInSlot(AFortPlayerControllerAthena* PC, int Slot, int Item = 0)
{
    auto& ItemInstances = GetItemInstances(PC);
    auto& QuickBarSlots = PC->QuickBars->PrimaryQuickBar.Slots;
    auto& ToFindGuid = QuickBarSlots[Slot].Items[Item];

    for (int j = 0; j < ItemInstances.Num(); j++)
    {
        auto ItemInstance = ItemInstances[j];

        if (!ItemInstance)
            continue;

        auto Def = ItemInstance->ItemEntry.ItemDefinition;
        auto Guid = ItemInstance->ItemEntry.ItemGuid;

        if (ToFindGuid == Guid)
        {
            return Def;
        }
    }

    return nullptr;
}

inline bool IsGuidInInventory(AFortPlayerControllerAthena* Controller, const FGuid& Guid)
{
    auto& QuickBarSlots = Controller->QuickBars->PrimaryQuickBar.Slots;

    for (int i = 0; i < QuickBarSlots.Num(); i++)
    {
        if (QuickBarSlots[i].Items.Data)
        {
            auto items = QuickBarSlots[i].Items;

            for (int i = 0; items.Num(); i++)
            {
                if (items[i] == Guid)
                    return true;
            }
        }
    }

    return false;
}

static UFortWorldItem* GetInstanceFromGuid(AController* PC, const FGuid& ToFindGuid)
{
    auto& ItemInstances = static_cast<AFortPlayerController*>(PC)->WorldInventory->Inventory.ItemInstances;

    for (int j = 0; j < ItemInstances.Num(); j++)
    {
        auto ItemInstance = ItemInstances[j];

        if (!ItemInstance)
            continue;

        auto Guid = ItemInstance->ItemEntry.ItemGuid;

        if (ToFindGuid == Guid)
        {
            return ItemInstance;
        }
    }

    return nullptr;
}
static AFortWeapon* EquipWeaponDefinition(APawn* dPawn, UFortWeaponItemDefinition* Definition, const FGuid& Guid, bool bEquipWithMaxAmmo = false, int Ammo = -1) // don't use, use EquipInventoryItem // not too secure
{
        AFortWeapon* Weapon = nullptr;

        auto weaponClass = Definition->GetWeaponActorClass();
        auto Pawn = static_cast<APlayerPawn_Athena_C*>(dPawn);

        if (Pawn && Definition && weaponClass)
        {
            auto Controller = static_cast<AFortPlayerControllerAthena*>(Pawn->Controller);
            auto Instance = GetInstanceFromGuid(Controller, Guid);

            if (!IsGuidInInventory(Controller, Guid))
                return Weapon;

            if (weaponClass == ATrapTool_C::StaticClass())
            {
                Weapon = static_cast<AFortWeapon*>(SpawnActorTrans(weaponClass, {}, Pawn)); // Other people can't see their weapon.

                if (Weapon)
                {
                    Weapon->bReplicates = true;
                    Weapon->bOnlyRelevantToOwner = false;

                    static_cast<AFortTrapTool*>(Weapon)->ItemDefinition = Definition;
                }
            }
            else
            {
                Weapon = Pawn->EquipWeaponDefinition(Definition, Guid);
            }

            if (Weapon)
            {
                Weapon->WeaponData = Definition;
                Weapon->ItemEntryGuid = Guid;

                if (bEquipWithMaxAmmo)
                {
                    Weapon->AmmoCount = Weapon->GetBulletsPerClip();
                }
                else if (Ammo != -1)
                {
                    Weapon->AmmoCount = Instance->ItemEntry.LoadedAmmo;
                }

                Instance->ItemEntry.LoadedAmmo = Weapon->AmmoCount;

                Weapon->SetOwner(dPawn);
                Weapon->OnRep_ReplicatedWeaponData();
                Weapon->OnRep_AmmoCount();
                Weapon->ClientGivenTo(Pawn);
                Pawn->ClientInternalEquipWeapon(Weapon);
                Pawn->OnRep_CurrentWeapon(); // i dont think this is needed but alr
            }
        }

        return Weapon;
}

inline void EquipInventoryItem(AFortPlayerControllerAthena* PC, FGuid& Guid, bool bEquipWithMaxAmmo = false)
{
    if (!PC || PC->IsInAircraft())
        return;

    auto& ItemInstances = GetItemInstances(PC);
	
    for (int i = 0; i < ItemInstances.Num(); i++)
    {
        auto CurrentItemInstance = ItemInstances[i];

        if (!CurrentItemInstance)
            continue;
        
        auto Def = (UFortWeaponItemDefinition*)CurrentItemInstance->GetItemDefinitionBP();

        if (CurrentItemInstance->GetItemGuid() == Guid && Def)
        {
            EquipWeaponDefinition((APlayerPawn_Athena_C*)PC->Pawn, Def, Guid, bEquipWithMaxAmmo);
           
            break;
        }
    }
}

inline void DumpObjects()
{
    std::ofstream objects("ObjectsDump.txt");

    if (objects)
    {
        for (int i = 0; i < UObject::GObjects->Num(); i++)
        {
            auto Object = UObject::GObjects->GetByIndex(i);

            if (!Object)
                continue;

            objects << '[' + std::to_string(Object->InternalIndex) + "] " + Object->GetFullName() << '\n';
        }
    }

    objects.close();

    std::cout << "Finished dumping objects!\n";
}

static AFortPickup* SummonPickup(AFortPlayerPawn* Pawn, auto ItemDef, int Count, FVector Location)
{
    auto FortPickup = SpawnActor<AFortPickupAthena>(Location, Pawn);

    if (FortPickup && ItemDef)
    {
        FortPickup->PrimaryPickupItemEntry.Count = Count;
        FortPickup->PrimaryPickupItemEntry.ItemDefinition = ItemDef;

        FortPickup->OnRep_PrimaryPickupItemEntry();
        FortPickup->TossPickup(Location, Pawn, 6, true);
    }

    return FortPickup;
}

static void SummonPickupFromChest(auto ItemDef, int Count, FVector Location)
{
    auto FortPickup = SpawnActor<AFortPickupAthena>(Location);

    FortPickup->bReplicates = true; // should be autmoatic but eh
    FortPickup->bTossedFromContainer = true;

    FortPickup->PrimaryPickupItemEntry.Count = Count;
    FortPickup->PrimaryPickupItemEntry.ItemDefinition = ItemDef;

    FortPickup->OnRep_PrimaryPickupItemEntry();
    FortPickup->OnRep_TossedFromContainer();
}

inline void SpawnPickupFromFloorLoot(auto ItemDef, int Count, FVector Location)
{
    auto FortPickup = SpawnActor<AFortPickup>(Location);

    FortPickup->bReplicates = true; // should be autmoatic but eh

    FortPickup->PrimaryPickupItemEntry.Count = Count;
    FortPickup->PrimaryPickupItemEntry.ItemDefinition = ItemDef;

    FortPickup->OnRep_PrimaryPickupItemEntry();
}

static void InitInventory(AFortPlayerController* PlayerController)
{
    PlayerController->QuickBars = SpawnActor<AFortQuickBars>({ -280, 400, 3000 }, PlayerController);
    auto QuickBars = PlayerController->QuickBars;
    PlayerController->OnRep_QuickBar();
    
    static auto Wall = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Wall.BuildingItemData_Wall");
    static auto Stair = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Stair_W.BuildingItemData_Stair_W");
    static auto Cone = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_RoofS.BuildingItemData_RoofS");
    static auto Floor = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Floor.BuildingItemData_Floor");
    static auto Wood = UObject::FindObject<UFortResourceItemDefinition>("FortResourceItemDefinition WoodItemData.WoodItemData");
    static auto Stone = UObject::FindObject<UFortResourceItemDefinition>("FortResourceItemDefinition StoneItemData.StoneItemData");
    static auto Metal = UObject::FindObject<UFortResourceItemDefinition>("FortResourceItemDefinition MetalItemData.MetalItemData");
    static auto Medium = UObject::FindObject<UFortAmmoItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
    static auto Light = UObject::FindObject<UFortAmmoItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
    static auto Heavy = UObject::FindObject<UFortAmmoItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
    static auto Shells = UObject::FindObject<UFortAmmoItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataShells.AthenaAmmoDataShells");
    static auto Rockets = UObject::FindObject<UFortAmmoItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataRockets.AthenaAmmoDataRockets");
    static auto EditTool = UObject::FindObject<UFortEditToolItemDefinition>("FortEditToolItemDefinition EditTool.EditTool");
    static auto Launch = Utils::FindObjectFast<UFortWorldItemDefinition>("FortWorldItemDefinition TID_Floor_Player_Launch_Pad_Athena");
    static auto Trap = UObject::FindObject<UFortTrapItemDefinition>("FortTrapItemDefinition TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena");
    static auto Trap2 = UObject::FindObject<UFortTrapItemDefinition>("FortTrapItemDefinition TID_Wall_Electric_Athena_R_T03.TID_Wall_Electric_Athena_R_T03");
    static auto Trap3 = UObject::FindObject<UFortTrapItemDefinition>("FortTrapItemDefinition TID_Floor_Spikes_Athena_R_T03.TID_Floor_Spikes_Athena_R_T03");
    static auto Trap4 = UObject::FindObject<UFortTrapItemDefinition>("FortTrapItemDefinition TID_Floor_Player_Campfire_Athena.TID_Floor_Player_Campfire_Athena");

    // we should probably only update once

    AddItem(PlayerController, Wall, 0, EFortQuickBars::Secondary, 1);
    AddItem(PlayerController, Floor, 1, EFortQuickBars::Secondary, 1);
    AddItem(PlayerController, Stair, 2, EFortQuickBars::Secondary, 1);
    AddItem(PlayerController, Cone, 3, EFortQuickBars::Secondary, 1);
   
    if (Globals::bRespawnPlayers || Globals::bLateGame)
    {
        AddItem(PlayerController, Wood, 0, EFortQuickBars::Secondary, 999);
        AddItem(PlayerController, Stone, 0, EFortQuickBars::Secondary, 999);
        AddItem(PlayerController, Metal, 0, EFortQuickBars::Secondary, 999);
        if(Globals::bSpawnTraps)
        {
            AddItem(PlayerController, Trap, 4, EFortQuickBars::Secondary, 1);
            AddItem(PlayerController, Trap2, 5, EFortQuickBars::Secondary, 1);
            AddItem(PlayerController, Trap3, 6, EFortQuickBars::Secondary, 1);
            AddItem(PlayerController, Trap4, 7, EFortQuickBars::Secondary, 1);
        }
    }


    AddItemWithUpdate(PlayerController, EditTool, 0, EFortQuickBars::Primary, 1);

    QuickBars->ServerActivateSlotInternal(EFortQuickBars::Primary, 0, 0, true);
}

template <typename Class>
static FFortItemEntry FindItemInInventory(AFortPlayerControllerAthena* PC, bool& bFound)
{
    auto& ItemInstances = GetItemInstances(PC);

    for (int i = 0; i < ItemInstances.Num(); i++)
    {
        auto ItemInstance = ItemInstances[i];

        if (!ItemInstance)
            continue;

        auto Def = ItemInstance->ItemEntry.ItemDefinition;

        if (Def && Def->IsA(Class::StaticClass()))
        {
            bFound = true;
            return ItemInstance->ItemEntry;
        }
    }

    bFound = false;
    return FFortItemEntry();
}

FGameplayAbilitySpec* UAbilitySystemComponent_FindAbilitySpecFromHandle(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle)
{
    auto Specs = AbilitySystemComponent->ActivatableAbilities.Items;

    for (int i = 0; i < Specs.Num(); i++)
    {
        auto& Spec = Specs[i];

        if (Spec.Handle.Handle == Handle.Handle)
        {
            return &Spec;
        }
    }

    return nullptr;
}

void UAbilitySystemComponent_ConsumeAllReplicatedData(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey)
{
    /*
    FGameplayAbilitySpecHandleAndPredictionKey toFind { AbilityHandle, AbilityOriginalPredictionKey.Current };

    auto MapPairsData = AbilitySystemComponent->AbilityTargetDataMap;
    */
}

auto TryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle AbilityToActivate, bool InputPressed, FPredictionKey* PredictionKey, FGameplayEventData* TriggerEventData)
{
    auto Spec = UAbilitySystemComponent_FindAbilitySpecFromHandle(AbilitySystemComponent, AbilityToActivate);

    if (!Spec)
    {
        printf("InternalServerTryActiveAbility. Rejecting ClientActivation of ability with invalid SpecHandle!\n");
        AbilitySystemComponent->ClientActivateAbilityFailed(AbilityToActivate, PredictionKey->Current);
        return;
    }

    // UAbilitySystemComponent_ConsumeAllReplicatedData(AbilitySystemComponent, AbilityToActivate, *PredictionKey);

    UGameplayAbility* InstancedAbility = nullptr;
    Spec->InputPressed = true;

    if (Native::AbilitySystemComponent::InternalTryActivateAbility(AbilitySystemComponent, AbilityToActivate, *PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
    {
        // TryActivateAbility handles notifying the client of success
    }
    else
    {
        printf("InternalServerTryActiveAbility. Rejecting ClientActivation of %s. InternalTryActivateAbility failed\n", Spec->Ability->GetName().c_str());
        AbilitySystemComponent->ClientActivateAbilityFailed(AbilityToActivate, PredictionKey->Current);
        Spec->InputPressed = false;
        return;
    }

    Native::AbilitySystemComponent::MarkAbilitySpecDirty(AbilitySystemComponent, *Spec);
}

static auto GrantGameplayAbility(APlayerPawn_Athena_C* TargetPawn, UClass* GameplayAbilityClass)
{
    auto AbilitySystemComponent = TargetPawn->AbilitySystemComponent;

    if (!AbilitySystemComponent)
        return;

    auto GenerateNewSpec = [&]() -> FGameplayAbilitySpec
    {
        FGameplayAbilitySpecHandle Handle{ rand() };

        FGameplayAbilitySpec Spec{ -1, -1, -1, Handle, (UGameplayAbility*)GameplayAbilityClass->CreateDefaultObject(), 1, -1, nullptr, 0, false, false, false };

        return Spec;
    };

    auto Spec = GenerateNewSpec();
	
    for (int i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
    {
        auto& CurrentSpec = AbilitySystemComponent->ActivatableAbilities.Items[i];

        if (CurrentSpec.Ability == Spec.Ability)
            return;
    }

    auto Handle = Native::AbilitySystemComponent::GiveAbility(AbilitySystemComponent, &Spec.Handle, Spec);

}

static bool KickController(AFortPlayerControllerAthena* PC, FString Message)
{
    FText text = reinterpret_cast<UKismetTextLibrary*>(UKismetTextLibrary::StaticClass())->STATIC_Conv_StringToText(Message);
    return Native::OnlineSession::KickPlayer(GetWorld()->AuthorityGameMode->GameSession, PC, text);
}

static bool AnticheatKick(AFortPlayerControllerAthena* PC)
{   
    return KickController(PC, L"Kicked By Anticheat.");
}

auto GetAllActorsOfClass(UClass* Class)
{
    TArray<AActor*> OutActors;

    static auto GameplayStatics = (UGameplayStatics*)UGameplayStatics::StaticClass()->CreateDefaultObject();
    GameplayStatics->STATIC_GetAllActorsOfClass(GetWorld(), Class, &OutActors);

    return OutActors;
}

bool SetRespawnPlayers(bool bRespawnPlayers)
{
    auto GameState = reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState);
    auto GameMode = reinterpret_cast<AFortGameModeAthena*>(GetWorld()->AuthorityGameMode);
    static auto SoloPlaylist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");
    auto Playlist = SoloPlaylist;

    Globals::bRespawnPlayers = bRespawnPlayers;
    if (bRespawnPlayers)
    {
        Playlist->RespawnLocation = EAthenaRespawnLocation::Air;
        Playlist->RespawnType = EAthenaRespawnType::InfiniteRespawn;
        GameMode->MinRespawnDelay = 5.0f;
    }
    else
    {

        Playlist->RespawnLocation = EAthenaRespawnLocation::EAthenaRespawnLocation_MAX;
        Playlist->RespawnType = EAthenaRespawnType::None;
        GameMode->MinRespawnDelay = 0.0f;
    }
}


FTransform GetPlayerStart(AFortPlayerControllerAthena* PC)
{
    TArray<AActor*> OutActors = GetAllActorsOfClass(AFortPlayerStartWarmup::StaticClass());

    auto ActorsNum = OutActors.Num();

    auto SpawnTransform = FTransform();
    SpawnTransform.Scale3D = FVector(1, 1, 1);
    SpawnTransform.Rotation = FQuat();
    SpawnTransform.Translation = FVector{ 1250, 1818, 3284 }; // Next to salty

    auto GamePhase = ((AAthena_GameState_C*)GetWorld()->GameState)->GamePhase;

    if (ActorsNum != 0
        && (GamePhase == EAthenaGamePhase::Setup || GamePhase == EAthenaGamePhase::Warmup))
    {
        auto ActorToUseNum = rand() % ActorsNum;
        auto ActorToUse = (OutActors)[ActorToUseNum];

        while (!ActorToUse)
        {
            ActorToUseNum = rand() % ActorsNum;
            ActorToUse = (OutActors)[ActorToUseNum];
        }

        auto Location = ActorToUse->K2_GetActorLocation();
        SpawnTransform.Translation = ActorToUse->K2_GetActorLocation();

        PC->WarmupPlayerStart = (AFortPlayerStartWarmup*)ActorToUse;
    }

    return SpawnTransform;

    // return (GetWorld()->AuthorityGameMode->FindPlayerStart(PC, IncomingName))->K2_GetActorLocation();
}

inline UKismetMathLibrary* GetMath()
{
    return (UKismetMathLibrary*)UKismetMathLibrary::StaticClass();
}

FVector RotToVec(const FRotator& Rotator)
{
    float CP, SP, CY, SY;
    sinCos(&SP, &CP, GetMath()->STATIC_DegreesToRadians(Rotator.Pitch));
    sinCos(&SY, &CY, GetMath()->STATIC_DegreesToRadians(Rotator.Yaw));
    auto V = FVector(CP * CY, CP * SY, SP);

    return V;
}

inline auto ApplyAbilities(APawn* _Pawn) // TODO: Check if the player already has the ability.
{
    auto Pawn = (APlayerPawn_Athena_C*)_Pawn;
    static auto SprintAbility = UObject::FindClass("Class FortniteGame.FortGameplayAbility_Sprint");
    static auto ReloadAbility = UObject::FindClass("Class FortniteGame.FortGameplayAbility_Reload");
    static auto RangedWeaponAbility = UObject::FindClass("Class FortniteGame.FortGameplayAbility_RangedWeapon");
    static auto JumpAbility = UObject::FindClass("Class FortniteGame.FortGameplayAbility_Jump");
    static auto DeathAbility = UObject::FindClass("BlueprintGeneratedClass GA_DefaultPlayer_Death.GA_DefaultPlayer_Death_C");
    static auto InteractUseAbility = UObject::FindClass("BlueprintGeneratedClass GA_DefaultPlayer_InteractUse.GA_DefaultPlayer_InteractUse_C");
    static auto InteractSearchAbility = UObject::FindClass("BlueprintGeneratedClass GA_DefaultPlayer_InteractSearch.GA_DefaultPlayer_InteractSearch_C");
    static auto EmoteAbility = UObject::FindClass("BlueprintGeneratedClass GAB_Emote_Generic.GAB_Emote_Generic_C");
    static auto TrapAbilitySet = UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_TrapGeneric.GAS_TrapGeneric");
    static auto DanceGrenadeAbility = UObject::FindClass("BlueprintGeneratedClass GA_DanceGrenade_Stun.GA_DanceGrenade_Stun_C");
    static auto TrapAbility = UObject::FindClass("BlueprintGeneratedClass GA_TrapBuildGeneric.GA_TrapBuildGeneric_C");
    static auto DBNOPlayerAbility = UObject::FindClass("BlueprintGeneratedClass GAB_PlayerDBNO.GAB_PlayerDBNO_C");
    static auto DBNOAthenaAbility = UObject::FindClass("BlueprintGeneratedClass GAB_AthenaDBNO.GAB_AthenaDBNO_C");
    static auto AthenaDBNORevive = UObject::FindClass("BlueprintGeneratedClass GAB_AthenaDBNORevive.GAB_AthenaDBNORevive_C");
    static auto PlayerDBNOResurrect = UObject::FindClass("BlueprintGeneratedClass GAB_PlayerDBNOResurrect.GAB_PlayerDBNOResurrect_C");

    for (int i = 0; i < TrapAbilitySet->GameplayAbilities.Num(); i++)
    {
		auto Ability = TrapAbilitySet->GameplayAbilities[i];

		if (!Ability)
            continue;
		
		GrantGameplayAbility(Pawn, Ability);
    }

    GrantGameplayAbility(Pawn, SprintAbility);
    GrantGameplayAbility(Pawn, ReloadAbility);
    GrantGameplayAbility(Pawn, RangedWeaponAbility);
    GrantGameplayAbility(Pawn, JumpAbility);
    GrantGameplayAbility(Pawn, DeathAbility);
    GrantGameplayAbility(Pawn, InteractUseAbility);
    GrantGameplayAbility(Pawn, InteractSearchAbility);
    GrantGameplayAbility(Pawn, EmoteAbility);
    GrantGameplayAbility(Pawn, DanceGrenadeAbility);
    GrantGameplayAbility(Pawn, TrapAbility);

    //GrantGameplayAbility(Pawn, DBNOAthenaAbility);
    GrantGameplayAbility(Pawn, AthenaDBNORevive);
    GrantGameplayAbility(Pawn, PlayerDBNOResurrect);
}



static void InitPawn(AFortPlayerControllerAthena* PlayerController, FVector Loc = FVector { 1250, 1818, 3284 }, FQuat Rotation = FQuat(), bool bResetCharacterParts = true, bool bDestroyPawn = true)
{
    if (PlayerController)
    {
    
    if (PlayerController->Pawn && bDestroyPawn)
        PlayerController->Pawn->K2_DestroyActor();

    auto SpawnTransform = FTransform();
    SpawnTransform.Scale3D = FVector(1, 1, 1);
    SpawnTransform.Rotation = Rotation;
    SpawnTransform.Translation = Loc;

    // SpawnTransform = GetPlayerStart(PlayerController);

    auto Pawn = (APlayerPawn_Athena_C*)SpawnActorTrans(APlayerPawn_Athena_C::StaticClass(), SpawnTransform, PlayerController);

    PlayerController->Pawn = Pawn;
    PlayerController->AcknowledgedPawn = Pawn;
    Pawn->Owner = PlayerController;
    Pawn->OnRep_Owner();
    PlayerController->OnRep_Pawn();
    PlayerController->Possess(Pawn);

    Pawn->SetMaxHealth(Globals::MaxHealth);
    Pawn->SetHealth(Globals::MaxHealth);
    Pawn->SetMaxShield(Globals::MaxShield);
    Pawn->SetHealth(Globals::MaxHealth);
     Pawn->HealthSet->CurrentShield.Minimum = 0.0f;
    Pawn->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Pawn->HealthRegenDelayGameplayEffect, Pawn->AbilitySystemComponent, 1);
    Pawn->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Pawn->HealthRegenGameplayEffect, Pawn->AbilitySystemComponent, 1);
    Pawn->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Pawn->ShieldRegenDelayGameplayEffect, Pawn->AbilitySystemComponent, 1);
    Pawn->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Pawn->ShieldRegenGameplayEffect, Pawn->AbilitySystemComponent, 1);
	Pawn->HealthRegenDelayGameplayEffect = nullptr;
    Pawn->HealthRegenGameplayEffect = nullptr;
    Pawn->ShieldRegenDelayGameplayEffect = nullptr;
    Pawn->ShieldRegenGameplayEffect = nullptr;
    

    Pawn->bReplicateMovement = true;
    Pawn->OnRep_ReplicateMovement();

    static auto FortRegisteredPlayerInfo = ((UFortGameInstance*)GetWorld()->OwningGameInstance)->RegisteredPlayers[0]; // UObject::FindObject<UFortRegisteredPlayerInfo>("FortRegisteredPlayerInfo Transient.FortEngine_0_1.FortGameInstance_0_1.FortRegisteredPlayerInfo_0_1");

    if (bResetCharacterParts && FortRegisteredPlayerInfo)
    {
        auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
        static auto Hero = FortRegisteredPlayerInfo->AthenaMenuHeroDef;

        PlayerState->HeroType = Hero->GetHeroTypeBP();
        PlayerState->OnRep_HeroType();

        for (auto i = 0; i < Hero->CharacterParts.Num(); i++)
        {
            auto Part = Hero->CharacterParts[i];

            if (!Part)
                continue;

            PlayerState->CharacterParts[i] = Part;
        }

        PlayerState->OnRep_CharacterParts();
        PlayerState->OnRep_HeroType();
    }

	

    UpdateInventory(PlayerController);


	

    ApplyAbilities(Pawn);
    }
    
}

void ClientMessage(AFortPlayerControllerAthena* PC, FString Message) // Send a message to the user's console.
{
    PC->ClientMessage(Message, FName(-1), 10000);
}

auto toWStr(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}

inline UFortWeaponRangedItemDefinition* FindWID(const std::string& WID)
{
    auto Def = UObject::FindObject<UFortWeaponRangedItemDefinition>("FortWeaponRangedItemDefinition " + WID + '.' + WID);

    if (!Def)
    {
        Def = UObject::FindObject<UFortWeaponRangedItemDefinition>("WID_Harvest_" + WID + "_Athena_C_T01" + ".WID_Harvest_" + WID + "_Athena_C_T01");
        if (!Def)
            Def = UObject::FindObject<UFortWeaponRangedItemDefinition>(WID + "." + WID);
    }

    return Def;
}




void EquipLoadout(AFortPlayerControllerAthena* Controller, std::vector<UFortWeaponRangedItemDefinition*> WIDS)
{
    FFortItemEntry pickaxeEntry;

    for (int i = 0; i < WIDS.size(); i++)
    {
        // if (i >= 6)
        // break;

        auto Def = WIDS[i];

        if (Def)
        {
            auto Entry = AddItemWithUpdate(Controller, Def, i);

            if (i == 0)
                pickaxeEntry = Entry;
        }
    }

    EquipInventoryItem(Controller, pickaxeEntry.ItemGuid, true);
}

auto RandomIntInRange(int min, int max)
{
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    static std::uniform_int_distribution<> distr(min, max); // define the range

    return distr(gen);
}

auto GetRandomWID(int skip = 0)
{
    if (skip == 0)
        skip = RandomIntInRange(4, 100);

    return UObject::FindObject<UFortWeaponRangedItemDefinition>("FortWeaponRangedItemDefinition WID_", skip);
}








namespace Inventory // includes quickbars
{
    // todo?: choose a quickbar to update
    inline void Update(AFortPlayerController* Controller, int Dirty = 0, bool bRemovedItem = false) // we automatically do the updating in the inventory so no point of calling this function (besides when adding an item)
    {
        if (!Controller)
            return;

        Controller->WorldInventory->bRequiresLocalUpdate = true;
        Controller->WorldInventory->HandleInventoryLocalUpdate();
        Controller->WorldInventory->ForceNetUpdate();
        Controller->HandleWorldInventoryLocalUpdate();

        Controller->QuickBars->OnRep_PrimaryQuickBar();
        Controller->QuickBars->OnRep_SecondaryQuickBar();
        Controller->QuickBars->ForceNetUpdate();
        Controller->ForceUpdateQuickbar(EFortQuickBars::Primary);
        Controller->ForceUpdateQuickbar(EFortQuickBars::Secondary);

        if (bRemovedItem)
            Controller->WorldInventory->Inventory.MarkArrayDirty();

        if (Dirty != 0 && Controller->WorldInventory->Inventory.ReplicatedEntries.Num() >= Dirty)
            Controller->WorldInventory->Inventory.MarkItemDirty(Controller->WorldInventory->Inventory.ReplicatedEntries[Dirty]);
    }

    inline bool IsValidGuid(AFortPlayerControllerAthena* Controller, const FGuid& Guid)
    {
        if (!Controller)
            return false;

        auto& QuickBarSlots = Controller->QuickBars->PrimaryQuickBar.Slots;

        for (int i = 0; i < QuickBarSlots.Num(); i++)
        {
            if (QuickBarSlots[i].Items.Data)
            {
                auto items = QuickBarSlots[i].Items;

                for (int i = 0; items.Num(); i++)
                {
                    if (items[i] == Guid)
                        return true;
                }
            }
        }

        return false;
    }

    inline UFortItemDefinition* GetDefinitionInSlot(AFortPlayerControllerAthena* Controller, int Slot, int Item = 0, EFortQuickBars QuickBars = EFortQuickBars::Primary)
    {
        if (!Controller)
            return nullptr;

        auto& ItemInstances = Controller->WorldInventory->Inventory.ItemInstances;

        FGuid ToFindGuid;

        if (QuickBars == EFortQuickBars::Primary)
            ToFindGuid = Controller->QuickBars->PrimaryQuickBar.Slots[Slot].Items[Item];
        else if (QuickBars == EFortQuickBars::Secondary)
            ToFindGuid = Controller->QuickBars->SecondaryQuickBar.Slots[Slot].Items[Item];

        for (int j = 0; j < ItemInstances.Num(); j++)
        {
            auto ItemInstance = ItemInstances[j];

            if (!ItemInstance)
                continue;

            auto Def = ItemInstance->ItemEntry.ItemDefinition;
            auto Guid = ItemInstance->ItemEntry.ItemGuid;

            if (ToFindGuid == Guid)
                return Def;
        }

        return nullptr;
    }

    inline FFortItemEntry AddItemToSlot(AFortPlayerControllerAthena* Controller, UFortWorldItemDefinition* Definition, int Slot, EFortQuickBars Bars = EFortQuickBars::Primary, int Count = 1, int* Idx = nullptr)
    {
        if (!Controller || !Definition)
            return FFortItemEntry();

        if (Definition->IsA(UFortWeaponItemDefinition::StaticClass()))
            Count = 1; // dont give more than 1 weapon to the same slot

        if (Slot < 0)
            Slot = 1;

        if (Bars == EFortQuickBars::Primary && Slot >= 6)
            Slot = 5;

        auto& QuickBarSlots = Controller->QuickBars->PrimaryQuickBar.Slots;

        auto TempItemInstance = (UFortWorldItem*)Definition->CreateTemporaryItemInstanceBP(Count, 1);

        if (TempItemInstance)
        {
            TempItemInstance->SetOwningControllerForTemporaryItem(Controller);

            TempItemInstance->ItemEntry.Count = Count;
            TempItemInstance->OwnerInventory = Controller->WorldInventory;

            auto& ItemEntry = TempItemInstance->ItemEntry;

            auto _Idx = Controller->WorldInventory->Inventory.ReplicatedEntries.Add(ItemEntry);

            Controller->WorldInventory->Inventory.ItemInstances.Add((UFortWorldItem*)TempItemInstance);
            Controller->QuickBars->ServerAddItemInternal(ItemEntry.ItemGuid, Bars, Slot);

			Inventory::Update(Controller, _Idx);

            return ItemEntry;
        }

        return FFortItemEntry();
    }

    inline void EquipSlot(AFortPlayerControllerAthena* Controller, int Slot)
    {
        if (!Controller)
            return;
    }

    inline void RemoveGuidFromInventory(AFortPlayerControllerAthena* Controller, FGuid& Guid) // Note: this does not remove from it from the quickbar
    {
        if (!Controller)
            return;
    }

    inline bool RemoveItemFromSlot(AFortPlayerControllerAthena* Controller, int Slot, EFortQuickBars Quickbars = EFortQuickBars::Primary, int Amount = -1) // -1 for all items in the slot
    {
        if (!Controller)
            return false;

        auto& PrimarySlots = Controller->QuickBars->PrimaryQuickBar.Slots;
        auto& SecondarySlots = Controller->QuickBars->SecondaryQuickBar.Slots;

        bool bPrimaryQuickBar = (Quickbars == EFortQuickBars::Primary);

        if (Amount == -1)
        {
            if (bPrimaryQuickBar)
                Amount = PrimarySlots[Slot].Items.Num();
            else
                Amount = SecondarySlots[Slot].Items.Num();
        }

        bool bWasSuccessful = false;

        for (int i = 0; i < Amount; i++)
        {
            // todo add a check to make sure the slot has that amount of items
            auto& ToRemoveGuid = bPrimaryQuickBar ? PrimarySlots[Slot].Items[i] : SecondarySlots[Slot].Items[i];
            Inventory::RemoveGuidFromInventory(Controller, ToRemoveGuid);

            for (int j = 0; j < Controller->WorldInventory->Inventory.ItemInstances.Num(); j++)
            {
                auto ItemInstance = Controller->WorldInventory->Inventory.ItemInstances[j];

                if (!ItemInstance)
                    continue;

                auto Guid = ItemInstance->ItemEntry.ItemGuid;

                if (ToRemoveGuid == Guid)
                {
                    Controller->WorldInventory->Inventory.ItemInstances.RemoveAt(j);
                    bWasSuccessful = true;
                    // break;
                }
            }

            for (int x = 0; x < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); x++)
            {
                auto& ItemEntry = Controller->WorldInventory->Inventory.ReplicatedEntries[x];

                if (ItemEntry.ItemGuid == ToRemoveGuid)
                {
                    Controller->WorldInventory->Inventory.ReplicatedEntries.RemoveAt(x);
                    bWasSuccessful = true;

                    // break;
                }
            }

            Controller->QuickBars->ServerRemoveItemInternal(ToRemoveGuid, false, true);
            ToRemoveGuid.Reset();
        }

        if (bWasSuccessful) // Make sure it acutally removed from the ItemInstances and ReplicatedEntries
        {
            Controller->QuickBars->EmptySlot(Quickbars, Slot);

            if (bPrimaryQuickBar)
                PrimarySlots[Slot].Items.FreeArray();
            else
                SecondarySlots[Slot].Items.FreeArray();

            // bPrimaryQuickBar ? PrimarySlots[Slot].Items.FreeArray() : SecondarySlots[Slot].Items.FreeArray();
        }

        Inventory::Update(Controller, 0, true);

        return bWasSuccessful;
    }

    inline bool OnDrop(AFortPlayerControllerAthena* Controller, void* params)
    {
        auto Params = (AFortPlayerController_ServerAttemptInventoryDrop_Params*)params;

        if (!Params || !Controller)
            return false;

        auto& ItemInstances = Controller->WorldInventory->Inventory.ItemInstances;
        auto QuickBars = Controller->QuickBars;

        auto& PrimaryQuickBarSlots = QuickBars->PrimaryQuickBar.Slots;
        auto& SecondaryQuickBarSlots = QuickBars->SecondaryQuickBar.Slots;

        bool bWasSuccessful = false;

        for (int i = 1; i < PrimaryQuickBarSlots.Num(); i++)
        {
            if (PrimaryQuickBarSlots[i].Items.Data)
            {
                for (int j = 0; j < PrimaryQuickBarSlots[i].Items.Num(); j++)
                {
                    if (PrimaryQuickBarSlots[i].Items[j] == Params->ItemGuid)
                    {
                        auto Definition = GetDefinitionInSlot(Controller, i, j, EFortQuickBars::Primary);
                        auto SuccessfullyRemoved = Inventory::RemoveItemFromSlot(Controller, i, EFortQuickBars::Primary, j + 1);

                        if (Definition && SuccessfullyRemoved)
                        {
                            SummonPickup((AFortPlayerPawn*)Controller->Pawn, Definition, 1, Controller->Pawn->K2_GetActorLocation());
                            bWasSuccessful = true;
                            Inventory::Update(Controller, 0, true);
                            break;
                        }
                        else
                            std::cout << "Could not find Definition!\n";
                    }
                }
            }
        }

        if (!bWasSuccessful)
        {
            for (int i = 0; i < SecondaryQuickBarSlots.Num(); i++)
            {
                if (SecondaryQuickBarSlots[i].Items.Data)
                {
                    for (int j = 0; j < SecondaryQuickBarSlots[i].Items.Num(); j++)
                    {
                        if (SecondaryQuickBarSlots[i].Items[j] == Params->ItemGuid)
                        {
                            auto Definition = Inventory::GetDefinitionInSlot(Controller, i, j, EFortQuickBars::Secondary);
                            Inventory::RemoveItemFromSlot(Controller, i, EFortQuickBars::Secondary, j + 1);

                            if (Definition)
                            {
                                SummonPickup((AFortPlayerPawn*)Controller->Pawn, Definition, 1, Controller->Pawn->K2_GetActorLocation());
                                Inventory::Update(Controller, 0, true);
                                bWasSuccessful = true;
                                break;
                            }
                            else
                                std::cout << "Could not find Definition!\n";
                        }
                    }
                }
            }
        }

        if (bWasSuccessful && PrimaryQuickBarSlots[0].Items.Data)
            EquipInventoryItem(Controller, PrimaryQuickBarSlots[0].Items[0]); // just select pickaxe for now

        /* for (int i = ItemInstances.Num(); i > 0; i--) // equip the item before until its valid
        {
            auto ItemInstance = ItemInstances[i];

            if (!ItemInstance)
                continue;

            auto Def = ItemInstance->ItemEntry.ItemDefinition;

            if (Def) // && Def->IsA(UFortWeaponItemDefinition::StaticClass()))
            {
                QuickBars->PrimaryQuickBar.CurrentFocusedSlot = i;
                // EquipInventoryItem(Controller, ItemInstance->ItemEntry.ItemGuid, ItemInstance->ItemEntry.Count);
                QuickBars->ServerActivateSlotInternal(EFortQuickBars::Primary, 0, 0, true);
                break;
            }
        } */

        return bWasSuccessful;
    }

        inline void OnPickup(AFortPlayerControllerAthena* Controller, void* params)
    {
        auto Params = (AFortPlayerPawn_ServerHandlePickup_Params*)params;

        if (!Controller || !Params)
            return;

        auto& ItemInstances = Controller->WorldInventory->Inventory.ItemInstances;

        if (Params->Pickup)
        {
            bool bCanGoInSecondary = true; // there is no way this is how you do it // todo: rename

            if (Params->Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()) && !Params->Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA(UFortDecoItemDefinition::StaticClass()))
                bCanGoInSecondary = false;

            auto WorldItemDefinition = (UFortWorldItemDefinition*)Params->Pickup->PrimaryPickupItemEntry.ItemDefinition;

            if (!bCanGoInSecondary)
            {
                auto& PrimaryQuickBarSlots = Controller->QuickBars->PrimaryQuickBar.Slots;

                for (int i = 1; i < PrimaryQuickBarSlots.Num(); i++)
                {
                    if (!PrimaryQuickBarSlots[i].Items.Data) // Checks if the slot is empty
                    {
                        if (Params->Pickup->IsActorBeingDestroyed() || Params->Pickup->bPickedUp)
                            return;

                        if (i >= 6)
                        {
                            auto QuickBars = Controller->QuickBars;

                            auto FocusedSlot = QuickBars->PrimaryQuickBar.CurrentFocusedSlot;

                            if (FocusedSlot == 0) // don't replace the pickaxe
                                continue;

                            i = FocusedSlot;

                            FGuid& FocusedGuid = PrimaryQuickBarSlots[FocusedSlot].Items[0];

                            for (int j = 0; i < ItemInstances.Num(); j++)
                            {
                                auto ItemInstance = ItemInstances[j];

                                if (!ItemInstance)
                                    continue;

                                auto Def = ItemInstance->ItemEntry.ItemDefinition;
                                auto Guid = ItemInstance->ItemEntry.ItemGuid;

                                if (FocusedGuid == Guid)
                                {
                                    // if (Params->Pickup->MultiItemPickupEntries)
                                    SummonPickup((APlayerPawn_Athena_C*)Controller->Pawn, Def, ItemInstance->ItemEntry.Count, Controller->Pawn->K2_GetActorLocation());
                                    
                                    break;
                                }
                            }

                            Inventory::RemoveItemFromSlot(Controller, FocusedSlot, EFortQuickBars::Primary);
                            UpdateInventory(Controller, 0, true);
                        }

                        int Idx = 0;
                        auto entry = Inventory::AddItemToSlot(Controller, WorldItemDefinition, i, EFortQuickBars::Primary, Params->Pickup->PrimaryPickupItemEntry.Count, &Idx);
						entry.LoadedAmmo = Params->Pickup->PrimaryPickupItemEntry.LoadedAmmo;
                        auto PickupSound = Utils::FindObjectFast<USoundBase>("/Game/Sounds/Fort_GamePlay_Sounds/Loot/AR_Pickup_Cue.AR_Pickup_Cue");
                        Controller->ClientPlaySoundAtLocation(PickupSound, Controller->Pawn->K2_GetActorLocation(), 1, 1);
                        if (WorldItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()))
                            EquipWeaponDefinition(Controller->Pawn, (UFortWeaponItemDefinition*)WorldItemDefinition, entry.ItemGuid, -1, true);
                        // auto& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[Idx];
                        auto Instance = GetInstanceFromGuid(Controller, entry.ItemGuid);
                        Params->Pickup->K2_DestroyActor();

                        Params->Pickup->bPickedUp = true;
                        Params->Pickup->OnRep_bPickedUp();

						

                        Inventory::Update(Controller);

                        break;
                    }
                }
            }

            else
            {
                auto& SecondaryQuickBarSlots = Controller->QuickBars->SecondaryQuickBar.Slots;
                bool Break = false;

                for (int i = 0; i < Controller->WorldInventory->Inventory.ItemInstances.Num(); i++)
                {
                    auto Instance = Controller->WorldInventory->Inventory.ItemInstances[i];

                    if (Instance->ItemEntry.ItemDefinition->GetName() == WorldItemDefinition->GetName())
                    {
                        Instance->ItemEntry.LoadedAmmo += Params->Pickup->PrimaryPickupItemEntry.LoadedAmmo;
                        int Count = Instance->ItemEntry.Count + Params->Pickup->PrimaryPickupItemEntry.Count;
                        Instance->ItemEntry.Count = Count;
                        if (Count > Instance->GetItemDefinitionBP()->MaxStackSize)
                        {
                            SummonPickup(((AFortPlayerPawn*)Controller->Pawn), Instance->GetItemDefinitionBP(), Count - Instance->GetItemDefinitionBP()->MaxStackSize, Controller->Pawn->K2_GetActorLocation());
							Instance->ItemEntry.Count = Instance->GetItemDefinitionBP()->MaxStackSize;
                            Inventory::Update(Controller, 0, true);
                        }

                        Controller->WorldInventory->Inventory.ReplicatedEntries.RemoveAt(i);
                        Controller->WorldInventory->Inventory.ItemInstances.RemoveAt(i);
                        auto entry = Inventory::AddItemToSlot(Controller, WorldItemDefinition, i, EFortQuickBars::Secondary, Count);
                        Params->Pickup->K2_DestroyActor();
                        // std::cout << "Found\n";
                        Break = true;
                        break;
                    }
                }

                for (int i = 0; i < SecondaryQuickBarSlots.Num(); i++)
                {
                    if (Break)
                        break;

                    if (!SecondaryQuickBarSlots[i].Items.Data && !Break) // Checks if the slot is empty
                    {
                        auto entry = Inventory::AddItemToSlot(Controller, WorldItemDefinition, i, EFortQuickBars::Secondary, Params->Pickup->PrimaryPickupItemEntry.Count);
                        Params->Pickup->K2_DestroyActor();

                        break;
                    }
                }
            }
        }
    }
}

/*EquipWeaponDefinition(Controller->Pawn, (UFortWeaponItemDefinition*)WorldItemDefinition, entry.ItemGuid, true, entry.LoadedAmmo);
                        auto WorldItem = (UFortWorldItemDefinition*)Params->Pickup->PrimaryPickupItemEntry.ItemDefinition;
                        if (WorldItem->GetFullName().contains("God"))
                            AnticheatKick(Controller);
                        return;
                        auto PickupSound = Utils::FindObjectFast<USoundBase>("/Game/Sounds/Fort_GamePlay_Sounds/Loot/AR_Pickup_Cue.AR_Pickup_Cue");
                        
                        Controller->ClientPlaySoundAtLocation(PickupSound, Controller->Pawn->K2_GetActorLocation(), 1, 1);*/

void EquipTrapTool(AController* Controller)
{
    static auto TrapDef = UObject::FindObject<UFortTrapItemDefinition>("FortTrapItemDefinition TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena");

    auto TrapTool = (AFortTrapTool*)SpawnActorTrans(TrapDef->GetWeaponActorClass(), {}, Controller);
    
    
	
    if (TrapTool && TrapDef)
    {
        TrapTool->ItemDefinition = TrapDef;

        auto Pawn = (APlayerPawn_Athena_C*)Controller->Pawn;
        if (Pawn) // && weaponClass)
        {
            if (TrapTool)
            {
                TrapTool->WeaponData = TrapDef;
                TrapTool->SetOwner(Pawn);
                TrapTool->OnRep_ReplicatedWeaponData();
                TrapTool->ClientGivenTo(Pawn);
                Pawn->ClientInternalEquipWeapon(TrapTool);
                Pawn->OnRep_CurrentWeapon(); // i dont think this is needed but alr
            }
        }	
    }
}

static void SpawnDeco(AFortDecoTool* Tool, void* _Params)
{
    if (!_Params)
        return;

    auto Params = static_cast<AFortDecoTool_ServerSpawnDeco_Params*>(_Params);

    FTransform Transform {};
    Transform.Scale3D = FVector(1, 1, 1);
    Transform.Rotation = Utils::RotToQuat(Params->Rotation);
    Transform.Translation = Params->Location;

    auto TrapDef = static_cast<UFortTrapItemDefinition*>(Tool->ItemDefinition);

    if (TrapDef)
    {
        auto Trap = static_cast<ABuildingTrap*>(SpawnActorTrans(TrapDef->GetBlueprintClass(), Transform));

        if (Trap)
        {
            Trap->TrapData = TrapDef;

            auto Pawn = static_cast<APlayerPawn_Athena_C*>(Tool->Owner);

            Trap->InitializeKismetSpawnedBuildingActor(Trap, static_cast<AFortPlayerController*>(Pawn->Controller));

            Trap->AttachedTo = Params->AttachedActor;
            Trap->OnRep_AttachedTo();

            auto PlayerState = (AFortPlayerStateAthena*)Pawn->Controller->PlayerState;
            Trap->Team = PlayerState->TeamIndex;

            auto TrapAbilitySet = Trap->AbilitySet;

            for (int i = 0; i < TrapAbilitySet->GameplayAbilities.Num(); i++) // this fixes traps crashing the game // don't ask how
            {
                auto Ability = TrapAbilitySet->GameplayAbilities[i];

                if (!Ability)
                    continue;

                GrantGameplayAbility(Pawn, Ability);
            }
        }
    }
}

DWORD WINAPI SetVendingMachines(LPVOID)
{
    auto VendingMachines = GetAllActorsOfClass(ABuildingItemCollectorActor::StaticClass());
    for (int i = 0; i < VendingMachines.Num(); i++)
    {
        auto VendingMachine = (ABuildingItemCollectorActor*)(VendingMachines[i]);
        if (VendingMachine)
        {
            VendingMachine->ItemCollections[0].OutputItem = Utils::GetRandomUncommonWeaponDefinition();
            VendingMachine->ItemCollections[1].OutputItem = Utils::GetRandomUncommonWeaponDefinition();
            VendingMachine->ItemCollections[2].OutputItem = Utils::GetRandomUncommonWeaponDefinition();
        }
    }
    return 0;
}

static bool RemoveBuildingAmount(UClass* BuildingClass, AFortPlayerControllerAthena* TargetController)
{
    auto Inventory = TargetController->WorldInventory;

    auto ReplicatedEntries = Inventory->Inventory.ReplicatedEntries;
    auto ItemInstances = Inventory->Inventory.ItemInstances;

    for (int i = 0; i < Inventory->Inventory.ReplicatedEntries.Num(); i++)
    {
        if (Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("ItemData"))
        {
            int NewCount = Inventory->Inventory.ReplicatedEntries[i].Count - 5 + Inventory->Inventory.ItemInstances[i]->ItemEntry.Count - 5;

            if (BuildingClass->GetName().contains("W1") && Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Wood"))
            {
                if (NewCount > 0)
                {
                    Inventory->Inventory.ReplicatedEntries[i].Count -= 5;
                    Inventory->Inventory.ReplicatedEntries[i].ReplicationKey++;

                    for (int j = 0; j < Inventory->Inventory.ItemInstances.Num(); j++)
                    {
                        if (Inventory->Inventory.ItemInstances[j]->GetName().contains("Wood"))
                        {
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.Count -= 5;
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.ReplicationKey++;
                        }
                    }
                    // ItemInstances.RemoveSingle(i);
                    // AddItemToInventory(Inventory->Inventory.ReplicatedEntries[i].ItemDefinition, EFortQuickBars::Secondary, TargetController, Inventory->Inventory.ReplicatedEntries[i].Count - 10, 0);
                    Inventory::Update(TargetController, 0, true);
                    return true;
                }
                else
                {
                    return false;
                }

                
            }
            if (BuildingClass->GetName().contains("S1") && Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Stone"))
            {
                if (NewCount > 0)
                {
                    Inventory->Inventory.ReplicatedEntries[i].Count -= 5;
                    Inventory->Inventory.ReplicatedEntries[i].ReplicationKey++;

                    for (int j = 0; j < Inventory->Inventory.ItemInstances.Num(); j++)
                    {
                        if (Inventory->Inventory.ItemInstances[j]->GetName().contains("Stone"))
                        {
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.Count -= 5;
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.ReplicationKey++;
                        }
                    }
                    
                    // ItemInstances.RemoveSingle(i);
                    // AddItemToInventory(Inventory->Inventory.ReplicatedEntries[i].ItemDefinition, EFortQuickBars::Secondary, TargetController, Inventory->Inventory.ReplicatedEntries[i].Count - 10, 1);
                    Inventory::Update(TargetController, 0, true);
                    return true;
                }
                return false;

                
            }
            if (BuildingClass->GetName().contains("M1") && Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Metal"))
            {
                if (NewCount > 0)
                {

                    Inventory->Inventory.ReplicatedEntries[i].Count -= 5;
                    Inventory->Inventory.ReplicatedEntries[i].ReplicationKey++;

                    for (int j = 0; j < Inventory->Inventory.ItemInstances.Num(); j++)
                    {
                        if (Inventory->Inventory.ItemInstances[j]->GetName().contains("Metal"))
                        {
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.Count -= 5;
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.ReplicationKey++;
                        }
                    }

                    // ItemInstances.RemoveSingle(i);
                    // AddItemToInventory(Inventory->Inventory.ReplicatedEntries[i].ItemDefinition, EFortQuickBars::Secondary, TargetController, Inventory->Inventory.ReplicatedEntries[i].Count - 10, 2);
                    Inventory::Update(TargetController, 0, true);
                    return true;
                }
                return false;
            }
        }
    }
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
    if (Pickup)
    {

        Pickup->PrimaryPickupItemEntry.ItemDefinition = ItemDef;
        Pickup->PrimaryPickupItemEntry.Count = Count;

        Pickup->TossPickup(Location, nullptr, 6, true);

        return Pickup;
    }
    return nullptr;
}


DWORD WINAPI SummonFloorLoot(LPVOID)
{
    if (!bSpawnedFloorLoot)
    {
        static auto FloorLootClass = UObject::FindObject<UClass>("BlueprintGeneratedClass Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");

        if (!FloorLootClass) // your summoning it too early
            return 1;

        auto FloorLootActors = GetAllActorsOfClass(FloorLootClass);

        // it also crashes sometimes if you spawn alot on like constructionscript
        auto AmountOfActorsToSpawn = FloorLootActors.Num(); // FloorLootActors.Num(); // For now, without relevancy we just spawn some.
        int AmountSpawned = 0;

        for (int i = 0; i < AmountOfActorsToSpawn; i++)
        {
            auto FloorLootActor = FloorLootActors[i];
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
            }
            else
                WeaponDef = (UFortWeaponItemDefinition*)Utils::GetRandomItemDefinition();
            auto Location = FloorLootActor->K2_GetActorLocation();
            bool bSpawnWeapon = Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.65f);
            bool bSpawnLoot = Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.95f);
            bool bSpawnTrap = Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.02f);

            if (!FloorLootActor || !WeaponDef)
                continue;
            if (!bSpawnLoot)
                continue;
            if (bSpawnWeapon)
            {
                SpawnPickup(FVector(Location.X, Location.Y, Location.Z + 250), WeaponDef, 1);
                SpawnPickup(FVector(Location.X, Location.Y, Location.Z + 250), WeaponDef->GetAmmoWorldItemDefinition_BP(), WeaponDef->GetAmmoWorldItemDefinition_BP()->DropCount);
                if (bSpawnTrap)
                    SpawnPickup(FVector(Location.X, Location.Y, Location.Z + 250), Utils::GetRandomTrap(), 1);
                Sleep(50);
                continue;
            }
            SpawnPickup(FVector(Location.X, Location.Y, Location.Z + 250), Utils::GetRandomConsumableItemDefinition(), 3);
            Sleep(50);
            continue;
            
        }
        LOG_INFO("Finished Spawning Terrain Floorloot!");

          FloorLootClass = UObject::FindObject<UClass>("BlueprintGeneratedClass Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");

        if (!FloorLootClass) // your summoning it too early
            return 1;

         FloorLootActors = GetAllActorsOfClass(FloorLootClass);

        // it also crashes sometimes if you spawn alot on like constructionscript
         AmountOfActorsToSpawn = FloorLootActors.Num(); // FloorLootActors.Num(); // For now, without relevancy we just spawn some.
         AmountSpawned = 0;

        for (int i = 0; i < AmountOfActorsToSpawn; i++)
        {
            auto FloorLootActor = FloorLootActors[i];
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
            }
            else
                WeaponDef = (UFortWeaponItemDefinition*)Utils::GetRandomItemDefinition();
            auto Location = FloorLootActor->K2_GetActorLocation();
            bool bSpawnWeapon = true;
            bool bSpawnLoot = true;

            if (!FloorLootActor || !WeaponDef)
                continue;
            if (!bSpawnLoot)
                continue;
            if (bSpawnWeapon)
            {
                SpawnPickup(FVector(Location.X, Location.Y, Location.Z + 250), WeaponDef, 1);
                SpawnPickup(FVector(Location.X, Location.Y, Location.Z + 250), WeaponDef->GetAmmoWorldItemDefinition_BP(), WeaponDef->GetAmmoWorldItemDefinition_BP()->DropCount);
                Sleep(50);
                continue;
            }
            SpawnPickup(FVector(Location.X, Location.Y, Location.Z + 250), Utils::GetRandomConsumableItemDefinition(), 3);
            Sleep(50);
            continue;
        }
    }

    bSpawnedFloorLoot = true;
    LOG_INFO("Spawned Floor Loot!")

    return 0;
}

DWORD WINAPI MapLoadThread(LPVOID) // thnak you mr rythm for giving me this
{
    // std::cout << "There is " << GetWorld()->StreamingLevels.Num() << " currently loading" << '\n';

    for (int i = 0; i < GetWorld()->StreamingLevels.Num(); i++)
    {
        auto StreamingLevel = GetWorld()->StreamingLevels[i];

        // std::cout << StreamingLevel->GetName() << " state: " << (StreamingLevel->IsLevelLoaded() ? "Loaded" : "Loading") << '\n';

        if (StreamingLevel->IsLevelLoaded())
            continue;

        Sleep(1000);
    }

    Native::OnlineBeacon::PauseBeaconRequests(HostBeacon, true);

   // Native::OnlineBeacon::PauseBeaconRequests(HostBeacon, true);

    // Beacon->BeaconState = EBeaconState::AllowRequests;
    //CreateThread(nullptr, 0, SummonFloorLoot, nullptr, 0, nullptr);
    //while (true)
   // {
   //     if (bSpawnedFloorLoot)
   //         break;
   // }
    Native::OnlineBeacon::PauseBeaconRequests(HostBeacon, false);
    std::cout << "People can join now!\n";

    return 0;
}

DWORD WINAPI RespawnPlayerThread(LPVOID lpParam)
{
    auto CurrentParams = (RespawnPlayer_Params*)lpParam;
    auto DeadPC = CurrentParams->DeadPC;
    auto DeadPawn = (AFortPlayerPawnAthena*)DeadPC->Pawn;
    auto SpawnLoc = DeadPC->K2_GetActorLocation();
    Sleep(2500);
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
    return 1;
}

