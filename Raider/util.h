#pragma once
#include "Globals.h"
#include "Logger.hpp"
#include <map>

static void Error(std::string error, bool bExit = false)
{
    // MessageBoxA(nullptr, error.c_str(), "Error", MB_OK | MB_ICONERROR);
    LOG_ERROR("{}", error.c_str());
    if (bExit)
        exit(0);
}

#define CheckNullFatal(x, y) \
    if (!x)                  \
    {                        \
        Error(y, true);      \
    }

template <typename T>
auto Merge(T v)
{
    return v;
}



template <typename T, typename... Types>
auto Merge(T var1, Types... vars)
{
    return reinterpret_cast<T*>(__int64(var1) + __int64(Merge(vars...)));
}

// Util macros
#define EXECUTE_ONE_TIME if (([] { 		    \
    static bool is_first_time = true;	    \
    auto was_first_time = is_first_time;    \
    is_first_time = false; 					\
    return was_first_time; }()))

// Hooking macros
#define DETOUR_START          \
    DetourTransactionBegin(); \
    DetourUpdateThread(GetCurrentThread());

#define DetourAttachE(Target, Detour) DetourAttach(&(void*&)Target, Detour);

#define DetourDetachE(Target, Detour) DetourDetach(reinterpret_cast<void**>(&Target), Detour);

#define DETOUR_END DetourTransactionCommit();

#define AddressToFunction(a, f) f = reinterpret_cast<decltype(f)>(a)

inline void SetupConsole()
{
    AllocConsole();

    FILE* pFile;
    freopen_s(&pFile, "CONOUT$", "w", stdout);
}



namespace Utils
{
     static __forceinline uintptr_t FindPattern(const char* signature, bool bRelative = false, uint32_t offset = 0)
    {
        uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
        static auto patternToByte = [](const char* pattern)
        {
            auto bytes = std::vector<int> {};
            const auto start = const_cast<char*>(pattern);
            const auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current)
            {
                if (*current == '?')
                {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.push_back(-1);
                }
                else
                {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
        const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

        const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = patternToByte(signature);
        const auto scanBytes = reinterpret_cast<std::uint8_t*>(base_address);

        const auto s = patternBytes.size();
        const auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i)
        {
            bool found = true;
            for (auto j = 0ul; j < s; ++j)
            {
                if (scanBytes[i + j] != d[j] && d[j] != -1)
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);
                if (bRelative)
                {
                    address = ((address + offset + 4) + *(int32_t*)(address + offset));
                    return address;
                }
                return address;
            }
        }
        return NULL;
    } 

    template <typename T>
    static T* FindObjectFast(std::string ObjectName, UClass* Class = UObject::StaticClass())
    {
        auto OrigInName = std::wstring(ObjectName.begin(), ObjectName.end()).c_str();

        static auto StaticFindObjectAddr = FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC 60 80 3D ? ? ? ? ? 45 0F B6 F1");
        auto StaticFindObject = (T * (*)(UClass*, UObject * Package, const wchar_t* OrigInName, bool ExactClass))(StaticFindObjectAddr);
        return StaticFindObject(Class, nullptr, OrigInName, false);
    }

	

    
    static float FRand()
    {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    bool GuidComp(FGuid guidA, FGuid guidB)
    {
        if (guidA.A == guidB.A && guidA.B == guidB.B && guidA.C == guidB.C && guidA.D == guidB.D)
            return true;
        else
            return false;
    }

    template <typename T>
    static T Max(T f, T s)
    {
        return f > s ? f : s;
    }

    std::vector<std::string> AthenaConsumables = {
        "/Game/Athena/Items/Consumables/Shields/Athena_Shields.Athena_Shields",
        "/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall",
        "/Game/Athena/Items/Consumables/PurpleStuff/Athena_PurpleStuff.Athena_PurpleStuff",
        "/Game/Athena/Items/Consumables/SuperMedkit/Athena_SuperMedkit.Athena_SuperMedkit",
        "/Game/Athena/Items/Consumables/Grenade/Athena_Grenade.Athena_Grenade",
        "/Game/Athena/Items/Consumables/KnockGrenade/Athena_KnockGrenade.Athena_KnockGrenade",
        "/Game/Athena/Items/Consumables/SmokeGrenade/Athena_SmokeGrenade.Athena_SmokeGrenade",
        "/Game/Athena/Items/Consumables/StickyGrenade/Athena_StickyGrenade.Athena_StickyGrenade",
        "/Game/Athena/Items/Consumables/Bush/Athena_Bush.Athena_Bush",
        "/Game/Athena/Items/Consumables/Medkit/Athena_Medkit.Athena_Medkit",
        "/Game/Athena/Items/Consumables/DanceGrenade/Athena_DanceGrenade.Athena_DanceGrenade",
        "/Game/Athena/Items/Consumables/SmokeGrenade/Athena_SmokeGrenade.Athena_SmokeGrenade"
    };

    std::vector<std::string> AthenaAssaultLootPool = {
        "/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_R_Ore_T03.WID_Assault_Auto_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_UC_Ore_T03.WID_Assault_Auto_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_C_Ore_T02.WID_Assault_Auto_Athena_C_Ore_T02",
        "/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_C_Ore_T02.WID_Assault_SemiAuto_Athena_C_Ore_T02",
        "/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_UC_Ore_T03.WID_Assault_SemiAuto_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_R_Ore_T03.WID_Assault_SemiAuto_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_LMG_Athena_VR_Ore_T03.WID_Assault_LMG_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_LMG_Athena_SR_Ore_T03.WID_Assault_LMG_Athena_SR_Ore_T03"
    };

    std::vector<std::string> AthenaShotgunLootPool = {
        "/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_UC_Ore_T03.WID_Shotgun_SemiAuto_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_R_Ore_T03.WID_Shotgun_SemiAuto_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_VR_Ore_T03.WID_Shotgun_SemiAuto_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SlugFire_Athena_VR.WID_Shotgun_SlugFire_Athena_VR",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SlugFire_Athena_SR.WID_Shotgun_SlugFire_Athena_SR"
    };

    std::vector<std::string> AthenaSmgLootPool = {
        "/Game/Athena/Items/Weapons/WID_Pistol_Scavenger_Athena_R_Ore_T03.WID_Pistol_Scavenger_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_Scavenger_Athena_VR_Ore_T03.WID_Pistol_Scavenger_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavy_Athena_C_Ore_T02.WID_Pistol_AutoHeavy_Athena_C_Ore_T02",
        "/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavy_Athena_UC_Ore_T03.WID_Pistol_AutoHeavy_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavy_Athena_R_Ore_T03.WID_Pistol_AutoHeavy_Athena_R_Ore_T03"
    };

    std::vector<std::string> AthenaPistolLootPool = {
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_C_Ore_T02.WID_Pistol_SemiAuto_Athena_C_Ore_T02",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_UC_Ore_T03.WID_Pistol_SemiAuto_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_R_Ore_T03.WID_Pistol_SemiAuto_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_VR_Ore_T03.WID_Pistol_SemiAuto_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_SR_Ore_T03.WID_Pistol_SemiAuto_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_C_Ore_T02.WID_Pistol_SixShooter_Athena_C_Ore_T02",
        "/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_UC_Ore_T03.WID_Pistol_SixShooter_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_R_Ore_T03.WID_Pistol_SixShooter_Athena_R_Ore_T03"
    };

    std::vector<std::string> AthenaSniperLootPool = {
        "/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Sniper_Standard_Scope_Athena_VR_Ore_T03.WID_Sniper_Standard_Scope_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Sniper_Standard_Scope_Athena_SR_Ore_T03.WID_Sniper_Standard_Scope_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_HandCannon_Athena_VR_Ore_T03.WID_Pistol_HandCannon_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_HandCannon_Athena_SR_Ore_T03.WID_Pistol_HandCannon_Athena_SR_Ore_T03"
    };

    std::vector<std::string> AthenaRocketLootPool = {
        "/Game/Athena/Items/Weapons/WID_Launcher_Rocket_Athena_R_Ore_T03.WID_Launcher_Rocket_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Launcher_Rocket_Athena_VR_Ore_T03.WID_Launcher_Rocket_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Launcher_Rocket_Athena_SR_Ore_T03.WID_Launcher_Rocket_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Launcher_Grenade_Athena_R_Ore_T03.WID_Launcher_Grenade_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Launcher_Grenade_Athena_VR_Ore_T03.WID_Launcher_Grenade_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Launcher_Grenade_Athena_SR_Ore_T03.WID_Launcher_Grenade_Athena_SR_Ore_T03"

    };

    std::vector<std::string> AthenaLootPool = {
        "/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_R_Ore_T03.WID_Assault_Auto_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_UC_Ore_T03.WID_Assault_Auto_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_C_Ore_T02.WID_Assault_Auto_Athena_C_Ore_T02",
        "/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_UC_Ore_T03.WID_Shotgun_SemiAuto_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_R_Ore_T03.WID_Shotgun_SemiAuto_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_VR_Ore_T03.WID_Shotgun_SemiAuto_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SlugFire_Athena_VR.WID_Shotgun_SlugFire_Athena_VR",
        "/Game/Athena/Items/Weapons/WID_Shotgun_SlugFire_Athena_SR.WID_Shotgun_SlugFire_Athena_SR",
        "/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Sniper_Standard_Scope_Athena_VR_Ore_T03.WID_Sniper_Standard_Scope_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Sniper_Standard_Scope_Athena_SR_Ore_T03.WID_Sniper_Standard_Scope_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_C_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_C_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_UC_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_Scavenger_Athena_UC_Ore_T03.WID_Pistol_Scavenger_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_Scavenger_Athena_R_Ore_T03.WID_Pistol_Scavenger_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_Scavenger_Athena_VR_Ore_T03.WID_Pistol_Scavenger_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_LMG_Athena_VR_Ore_T03.WID_Assault_LMG_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Assault_LMG_Athena_SR_Ore_T03.WID_Assault_LMG_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_C_Ore_T02.WID_Pistol_SemiAuto_Athena_C_Ore_T02",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_UC_Ore_T03.WID_Pistol_SemiAuto_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_R_Ore_T03.WID_Pistol_SemiAuto_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_VR_Ore_T03.WID_Pistol_SemiAuto_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_SR_Ore_T03.WID_Pistol_SemiAuto_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_C_Ore_T02.WID_Pistol_SixShooter_Athena_C_Ore_T02",
        "/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_UC_Ore_T03.WID_Pistol_SixShooter_Athena_UC_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_R_Ore_T03.WID_Pistol_SixShooter_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_HandCannon_Athena_VR_Ore_T03.WID_Pistol_HandCannon_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Pistol_HandCannon_Athena_SR_Ore_T03.WID_Pistol_HandCannon_Athena_SR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Launcher_Rocket_Athena_R_Ore_T03.WID_Launcher_Rocket_Athena_R_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Launcher_Rocket_Athena_VR_Ore_T03.WID_Launcher_Rocket_Athena_VR_Ore_T03",
        "/Game/Athena/Items/Weapons/WID_Launcher_Rocket_Athena_SR_Ore_T03.WID_Launcher_Rocket_Athena_SR_Ore_T03"
    };

    std::vector<std::string> TrapsPool = {
        "/Game/Athena/Items/Traps/TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena",
        "/Game/Athena/Items/Traps/TID_Floor_Spikes_Athena_R_T03.TID_Floor_Spikes_Athena_R_T03"
    };

    std::vector<std::string> ResourcePool = {
        "/Game/Items/ResourcePickups/WoodItemData.WoodItemData",
        "/Game/Items/ResourcePickups/StoneItemData.StoneItemData",
        "/Game/Items/ResourcePickups/MetalItemData.MetalItemData"
    };

    std::vector<std::string> AmmoPool = {
        "/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets",
        "/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells",
        "/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium",
        "/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight",
        "/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy"
    };

    std::vector<std::string> PickaxePool = {
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Anchor_Athena.WID_Harvest_Pickaxe_Anchor_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_ArtDeco.WID_Harvest_Pickaxe_ArtDeco",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Assassin.WID_Harvest_Pickaxe_Assassin",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_BoltOn_Athena_C_T01.WID_Harvest_Pickaxe_BoltOn_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Brite.WID_Harvest_Pickaxe_Brite",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Carrot.WID_Harvest_Pickaxe_Carrot",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_CircuitBreaker.WID_Harvest_Pickaxe_CircuitBreaker",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_CuChulainn.WID_Harvest_Pickaxe_CuChulainn",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Deathvalley_Athena_C_T01.WID_Harvest_Pickaxe_Deathvalley_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Disco_Athena.WID_Harvest_Pickaxe_Disco_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_District_Athena.WID_Harvest_Pickaxe_District_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Dragon.WID_Harvest_Pickaxe_Dragon",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Flamingo_Athena_C_T01.WID_Harvest_Pickaxe_Flamingo_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Heart_Athena.WID_Harvest_Pickaxe_Heart_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_HolidayCandyCane_Athena.WID_Harvest_Pickaxe_HolidayCandyCane_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_HolidayGiftWrap_Athena.WID_Harvest_Pickaxe_HolidayGiftWrap_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_IcePick_Athena_C_T01.WID_Harvest_Pickaxe_IcePick_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Keg_Athena.WID_Harvest_Pickaxe_Keg_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Lockjaw_Athena_C_T01.WID_Harvest_Pickaxe_Lockjaw_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Medieval_Athena.WID_Harvest_Pickaxe_Medieval_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Megalodon_Athena.WID_Harvest_Pickaxe_Megalodon_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_PajamaParty.WID_Harvest_Pickaxe_PajamaParty",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Pizza.WID_Harvest_Pickaxe_Pizza",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Plunger.WID_Harvest_Pickaxe_Plunger",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_PotOfGold.WID_Harvest_Pickaxe_PotOfGold",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Prismatic.WID_Harvest_Pickaxe_Prismatic",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_RockerPunk.WID_Harvest_Pickaxe_RockerPunk",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Scavenger.WID_Harvest_Pickaxe_Scavenger",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Shark_Athena.WID_Harvest_Pickaxe_Shark_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_SickleBat_Athena_C_T01.WID_Harvest_Pickaxe_SickleBat_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_SkiBoot.WID_Harvest_Pickaxe_SkiBoot",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Smiley_Athena_C_T01.WID_Harvest_Pickaxe_Smiley_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Space.WID_Harvest_Pickaxe_Space",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Spikey_Athena_C_T01.WID_Harvest_Pickaxe_Spikey_Athena_C_T01",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Squeak.WID_Harvest_Pickaxe_Squeak",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Stealth.WID_Harvest_Pickaxe_Stealth",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Tactical.WID_Harvest_Pickaxe_Tactical",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_TacticalBlack.WID_Harvest_Pickaxe_TacticalBlack",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_TacticalUrban.WID_Harvest_Pickaxe_TacticalUrban",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Teslacoil_Athena.WID_Harvest_Pickaxe_Teslacoil_Athena",
        "/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_WinterCamo_Athena.WID_Harvest_Pickaxe_WinterCamo_Athena"
    };

    
    static UFortItemDefinition* GetRandomPickaxe()
    {
        while (true)
        {
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(PickaxePool.size());
            auto Item = PickaxePool[Idx];
            auto Def = Utils::FindObjectFast<UFortItemDefinition>(Item);
            if (!Def)
                continue;
            auto rng = std::default_random_engine {};
            std::shuffle(PickaxePool.begin(), PickaxePool.end(), rng);
            return Def;
        }
    }

    static UFortItemDefinition* GetRandomItemDefinition()
    {
        while (true)
        {
            if (Globals::MathLibrary)
            {
                std::cout << "Math Library is Valid!" << std::endl;
            }
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(AthenaLootPool.size());
            std::cout << "STATIC_RandomInteger Called! Idx set!" << std::endl;
            auto Item = AthenaLootPool[Idx];
            std::cout << "Item name found! Item Name: " << Item << std::endl;
            auto Def = Utils::FindObjectFast<UFortItemDefinition>(Item);
            if (!Def)
                continue;

            std::cout << "Item Def is valid!, Item Name: " << Def->GetName() << std::endl;
            auto rng = std::default_random_engine {};
            std::cout << "rng set!" << std::endl;
            std::shuffle(AthenaLootPool.begin(), AthenaLootPool.end(), rng);
            std::cout << "Loot pool shuffled." << std::endl;
            return Def;
        }
    }


    static std::vector<std::string> CalculateLootType()
    {
        if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.43))
            return AthenaAssaultLootPool;
        if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.22))
            return AthenaShotgunLootPool;
        if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.14))
            return AthenaSmgLootPool;
        if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.11))
            return AthenaPistolLootPool;
        if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.10))
            return AthenaSniperLootPool;
        if (Globals::MathLibrary->STATIC_RandomBoolWithWeight(0.7))
            return AthenaRocketLootPool;
        return AthenaAssaultLootPool;
    }
    

    static UFortWeaponItemDefinition* GetRandomGoldWeaponDefinition()
    {
        while (true)
        {
            auto LootPool = CalculateLootType();
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(LootPool.size());
            auto Item = LootPool[Idx];
            auto Def = Utils::FindObjectFast<UFortWeaponItemDefinition>(Item);
            if (!Def || Def->Rarity != EFortRarity::Fine)
                continue;

            auto rng = std::default_random_engine {};
            std::shuffle(LootPool.begin(), LootPool.end(), rng);
            return Def;
        }
    }

    static UFortWeaponItemDefinition* GetRandomEpicWeaponDefinition()
    {
        while(true)
        {
            auto LootPool = CalculateLootType();
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(LootPool.size());
            auto Item = LootPool[Idx];
			auto Def = Utils::FindObjectFast<UFortWeaponItemDefinition>(Item);
            if (!Def || Def->Rarity != EFortRarity::Quality)
                continue;
            auto rng = std::default_random_engine {};
            std:shuffle(LootPool.begin(), LootPool.end(), rng);
            return Def;
            
        }
    }

    static UFortWeaponItemDefinition* GetRandomRareWeaponDefinition()
    {
        while(true)
        {
            auto LootPool = CalculateLootType();
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(LootPool.size());
            auto Item = LootPool[Idx];
            auto Def = Utils::FindObjectFast<UFortWeaponItemDefinition>(Item);
            if (!Def || Def->Rarity != EFortRarity::Sturdy)
                continue;
            auto rng = std::default_random_engine {};
        std:shuffle(LootPool.begin(), LootPool.end(), rng);
            return Def;
        }
    }

    static UFortWeaponItemDefinition* GetRandomUncommonWeaponDefinition()
    {
        while (true)
        {
            auto LootPool = CalculateLootType();
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(LootPool.size());
            auto Item = LootPool[Idx];
            auto Def = Utils::FindObjectFast<UFortWeaponItemDefinition>(Item);
            if (!Def || Def->Rarity != EFortRarity::Ordinary)
                continue;
            auto rng = std::default_random_engine {};
        std:
            shuffle(LootPool.begin(), LootPool.end(), rng);
            return Def;
        }
    }

    static UFortWeaponItemDefinition* GetRandomCommonWeaponDefinition()
    {
        while (true)
        {
            auto LootPool = CalculateLootType();
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(LootPool.size());
            auto Item = LootPool[Idx];
            auto Def = Utils::FindObjectFast<UFortWeaponItemDefinition>(Item);
            if (!Def || Def->Rarity != EFortRarity::Handmade)
                continue;
            auto rng = std::default_random_engine {};
        std:
            shuffle(LootPool.begin(), LootPool.end(), rng);
            return Def;
        }
    }

    static UFortItemDefinition* GetRandomConsumableItemDefinition()
    {
        while (true)
        {
            if (Globals::MathLibrary)
            {
                std::cout << "Math Library is Valid!" << std::endl;
            }
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(AthenaConsumables.size());
            std::cout << "STATIC_RandomInteger Called! Idx set!" << std::endl;
            auto Item = AthenaConsumables[Idx];
            std::cout << "Item name found! Item Name: " << Item << std::endl;
            auto Def = Utils::FindObjectFast<UFortItemDefinition>(Item);
            if (!Def)
                continue;

            std::cout << "Item Def is valid!, Item Name: " << Def->GetName() << std::endl;
            auto rng = std::default_random_engine {};
            std::cout << "rng set!" << std::endl;
            std::shuffle(AthenaConsumables.begin(), AthenaConsumables.end(), rng);
            std::cout << "Loot pool shuffled." << std::endl;
            return Def;
        }
    }

    static UFortItemDefinition* GetRandomResourceItemDefinition()
    {
        while (true)
        {
            if (Globals::MathLibrary)
            {
                std::cout << "Math Library is Valid!" << std::endl;
            }
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(ResourcePool.size());
            std::cout << "STATIC_RandomInteger Called! Idx set!" << std::endl;
            auto Item = ResourcePool[Idx];
            std::cout << "Item name found! Item Name: " << Item << std::endl;
            auto Def = Utils::FindObjectFast<UFortItemDefinition>(Item);
            if (!Def)
                continue;

            std::cout << "Item Def is valid!, Item Name: " << Def->GetName() << std::endl;
            auto rng = std::default_random_engine {};
            std::cout << "rng set!" << std::endl;
            std::shuffle(ResourcePool.begin(), ResourcePool.end(), rng);
            std::cout << "Loot pool shuffled." << std::endl;
            return Def;
        }
    }

    static UFortAmmoItemDefinition* GetRandomAmmoItemDefinition()
    {
        while (true)
        {
            auto Idx = Globals::MathLibrary->STATIC_RandomInteger(AmmoPool.size());
            auto Item = AmmoPool[Idx];
            auto Def = Utils::FindObjectFast<UFortAmmoItemDefinition>(Item);
            if (!Def)
                continue;

            auto rng = std::default_random_engine {};
            std::shuffle(AmmoPool.begin(), AmmoPool.end(), rng);
            return Def;
        }
    }

    auto GetDeathCause(FFortPlayerDeathReport DeathReport)
    {
        static std::map<std::string, EDeathCause> DeathCauses {
            { "weapon.ranged.shotgun", EDeathCause::Shotgun },
            { "weapon.ranged.assault", EDeathCause::Rifle },
            { "Gameplay.Damage.Environment.Falling", EDeathCause::FallDamage },
            { "weapon.ranged.sniper", EDeathCause::Sniper },
            { "Weapon.Ranged.SMG", EDeathCause::SMG },
            { "weapon.ranged.heavy.rocket_launcher", EDeathCause::RocketLauncher },
            { "weapon.ranged.heavy.grenade_launcher", EDeathCause::GrenadeLauncher },
            { "Weapon.ranged.heavy.grenade", EDeathCause::Grenade },
            { "Weapon.Ranged.Heavy.Minigun", EDeathCause::Minigun },
            { "Weapon.Ranged.Crossbow", EDeathCause::Bow },
            { "trap.floor", EDeathCause::Trap },
            { "weapon.ranged.pistol", EDeathCause::Pistol },
            { "Gameplay.Damage.OutsideSafeZone", EDeathCause::OutsideSafeZone },
            { "Weapon.Melee.Impact.Pickaxe", EDeathCause::Melee }
        };

        for (int i = 0; i < DeathReport.Tags.GameplayTags.Num(); i++)
        {
            auto TagName = DeathReport.Tags.GameplayTags[i].TagName.ToString();

            for (auto Map : DeathCauses)
            {
                if (TagName == Map.first)
                    return Map.second;
                else
                    continue;
            }
        }

        return EDeathCause::Unspecified;
    }
};
