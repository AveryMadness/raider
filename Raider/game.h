#pragma once

#include "ue4.h"

namespace Game
{
    void Start()
    {
        GetPlayerController()->SwitchLevel(L"Athena_Terrain?game=/Game/Athena/Athena_GameMode.Athena_GameMode_C");
        bTraveled = true;
    }

    void OnReadyToStartMatch()
    {
        //Set global variables
        Globals::bDBNO = false;
        Globals::bFriendlyFire = true;
        Globals::bLargeTeamGame = false;
        Globals::bRespawnPlayers = false;
        Globals::MaxHealth = 100;
        Globals::MaxShield = 100;
		
        auto GameState = reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState);
        auto GameMode = reinterpret_cast<AAthena_GameMode_C*>(GetWorld()->AuthorityGameMode);
        static auto SoloPlaylist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");
        static auto DuoPlaylist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultDuo.Playlist_DefaultDuo");
        auto InProgress = UObject::FindObject<UKismetStringLibrary>("Default__KismetStringLibrary")->STATIC_Conv_StringToName(L"InProgress");

        GameState->bGameModeWillSkipAircraft = true;
        GameState->AircraftStartTime = 9999.9f;
        GameState->WarmupCountdownEndTime = 99999.9f;

        GameState->GamePhase = EAthenaGamePhase::Warmup;
        GameState->OnRep_GamePhase(EAthenaGamePhase::None);

        GameMode->bDisableGCOnServerDuringMatch = true;
        GameMode->bAllowSpectateAfterDeath = true;

        GameMode->MatchState = InProgress;
        GameMode->K2_OnSetMatchState(InProgress);

        auto Playlist = SoloPlaylist;

        if (Playlist)
        {
            Playlist->bNoDBNO = !Globals::bDBNO;
            Playlist->bIsLargeTeamGame = Globals::bLargeTeamGame;
            if (Globals::bRespawnPlayers)
            {
                Playlist->RespawnLocation = EAthenaRespawnLocation::Air;
                Playlist->RespawnType = EAthenaRespawnType::InfiniteRespawn;
                
            }
            if (Globals::bFriendlyFire)
            {
                Playlist->FriendlyFireType = EFriendlyFireType::On;
                GameMode->FriendlyFireType = EFriendlyFireType::On;
            }

            GameState->CurrentPlaylistData = Playlist;
            GameState->OnRep_CurrentPlaylistData();
        }

        

        GameMode->StartPlay();

        GameState->bReplicatedHasBegunPlay = true;
        GameState->OnRep_ReplicatedHasBegunPlay();

        GameMode->StartMatch();
        GameMode->bAlwaysDBNO = Globals::bDBNO;
        GameMode->MinRespawnDelay = 5.0f;

		// https://github.com/EpicGames/UnrealEngine/blob/46544fa5e0aa9e6740c19b44b0628b72e7bbd5ce/Engine/Source/Runtime/Engine/Private/ActorReplication.cpp#L300
        // By default the NetCullDistanceSquared is very low, I don't know why.
        GetWorld()->NetworkManager->NetCullDistanceSquared *= 3;// 2.5;
    }
}
