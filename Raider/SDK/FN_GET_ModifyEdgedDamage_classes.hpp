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

// BlueprintGeneratedClass GET_ModifyEdgedDamage.GET_ModifyEdgedDamage_C
// 0x0000 (0x0670 - 0x0670)
class UGET_ModifyEdgedDamage_C : public UGET_ModifyPhysicalDamage_C
{
public:

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("BlueprintGeneratedClass GET_ModifyEdgedDamage.GET_ModifyEdgedDamage_C");
		return ptr;
	}

};


}

#ifdef _MSC_VER
	#pragma pack(pop)
#endif
