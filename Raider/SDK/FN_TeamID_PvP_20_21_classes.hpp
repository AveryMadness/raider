#pragma once

// Fortnite (3.1) SDK

#ifdef _MSC_VER
	#pragma pack(push, 0x8)
#endif

#include "../SDK.hpp"

namespace SDK
{
//---------------------------------------------------------------------------
//Classes
//---------------------------------------------------------------------------

// BlueprintGeneratedClass TeamID_PvP_20_21.TeamID_PvP_20_C
// 0x0000 (0x0038 - 0x0038)
class UTeamID_PvP_20_C : public UFortTeamIdentification
{
public:

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("BlueprintGeneratedClass TeamID_PvP_20_21.TeamID_PvP_20_C");
		return ptr;
	}

};


}

#ifdef _MSC_VER
	#pragma pack(pop)
#endif
