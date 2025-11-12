# Integrating TeamGlicko2 Rating System with UT Team Showdown

## Overview

This document explains how to integrate the TeamGlicko2 C++ rating system with **Unreal Tournament 4's Team Showdown** game mode to automatically track and update player ratings **after each elimination round**.

### Key Facts

- **Target Game Mode**: Team Showdown (`AUTTeamShowdownGame`) - UT4's competitive elimination mode
- **Update Frequency**: **Per-round**, not per-match
- **Round Duration**: 2-3 minutes per round
- **Match Structure**: First team to win N rounds (e.g., 5) wins the match

### Why Round-Based Updates?

Team Showdown is structured as **multiple elimination rounds** within a single match. Updating ratings after each round provides:

1. **Frequent Feedback**: Players see skill progression every 2-3 minutes
2. **Performance Recognition**: Carrying your team in a crucial round is immediately rewarded
3. **Reduced Variance**: Multiple data points per match = faster rating convergence
4. **Better Accuracy**: Per-round performance is more meaningful than aggregate match stats

## Game Mode Architecture

### Showdown Hierarchy

```
AUTBaseGameMode
└── AUTGameMode
    └── AUTDuelGame
        └── AUTShowdownGame (1v1 base)
            └── AUTTeamShowdownGame (NvsN team elimination)
```

**Key Files**:
- `UnrealTournament/Source/UnrealTournament/Public/UTTeamShowdownGame.h`
- `UnrealTournament/Source/UnrealTournament/Private/UTTeamShowdownGame.cpp`
- `UnrealTournament/Source/UnrealTournament/Public/UTShowdownGame.h`
- `UnrealTournament/Source/UnrealTournament/Private/UTShowdownGame.cpp`

### Round Lifecycle

```
Match Start
├── Round 1
│   ├── Spawn Selection Phase (SpawnSelectionTime seconds)
│   ├── Combat Phase (TimeLimit per round)
│   ├── Round End (one team eliminated OR time expires)
│   ├── Intermission (1-2 seconds)
│   └── **[RATING UPDATE HOOK POINT]**
├── Round 2
│   └── ... (repeat)
└── Match End (one team reaches GoalScore)
```

### Round End Detection

**Code Location**: `UTTeamShowdownGame.cpp:194` (`ScoreKill_Implementation`)

**Round ends when**:
1. **Elimination**: Last player on a team dies
   ```cpp
   if (AliveCount == 0)  // No players alive on losing team
   {
       KillerTeam->Score += 1;
       LastRoundWinner = KillerTeam;
       SetTimerUFunc(this, FName(TEXT("StartIntermission")), 1.0f, false);
   }
   ```

2. **Time Expiration**: Round timer reaches zero
   ```cpp
   // UTShowdownGame.cpp:425 - CheckGameTime()
   if (TimeLimit > 0 && UTGameState->GetRemainingTime() <= 0)
   {
       ScoreExpiredRoundTime();  // Determine winner via tiebreak
       SetTimerUFunc(this, FName(TEXT("StartIntermission")), 2.0f, false);
   }
   ```

### Tiebreak Rules

**Code Location**: `UTTeamShowdownGame.cpp:312` (`GetTiebreakWinner`)

When time expires, winner determined by:
1. Most players alive
2. If tied, most total health
3. If still tied, both teams get a point (draw)

### Integration Hook Point

**Code Location**: `UTShowdownGame.cpp:436` (`StartIntermission`)

**What happens**:
1. Award survival bonus (+50 score to winning team)
2. Check if match should end (team reached GoalScore)
3. **[INSERT RATING UPDATE HERE]**
4. Set `MatchState::MatchIntermission`
5. After intermission → `HandleMatchIntermission()` → `StartNewRound()`

### Available Statistics Per Round

**From `AUTPlayerState`** (tracked per round):
- `RoundKills`: Eliminations this round
- `RoundDeaths`: Deaths this round
- `RoundDamageDone`: Damage dealt this round
- `bOutOfLives`: Whether player is eliminated
- `Team`: Team membership (`AUTTeamInfo*`)

**Additional stats** (can be added):
- First blood
- Multi-kills
- Survival time
- Damage taken
- Assists

## Integration Options

### Option 1: Direct C++ Integration (Recommended)

Compile the TeamGlicko2 system directly into the UnrealTournament module and call it from game mode.

#### Step 1: Add ElimRank to Build System

**File: `UnrealTournament/Source/UnrealTournament/UnrealTournament.Build.cs`**

```csharp
public class UnrealTournament : ModuleRules
{
    public UnrealTournament(TargetInfo Target)
    {
        // ... existing code ...
        
        // Add ElimRank include path (relative to UnrealTournament/Source/)
        PublicIncludePaths.AddRange(
            new string[] {
                // ... existing paths ...
                "../ElimRank/include"  // Points to ElimRank/include from UnrealTournament/Source
            }
        );
        
        // ... rest of build config ...
    }
}
```

#### Step 2: Create UObject Wrapper

Create a UObject wrapper to interface between Unreal Engine and the C++ rating system.

**File: `UnrealTournament/Source/UnrealTournament/Public/UTRatingManager.h`**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TeamGlicko2System.h"
#include "UTRatingManager.generated.h"

/**
 * Wrapper for TeamGlicko2 rating system
 * Manages player rating storage and match processing
 */
UCLASS()
class UNREALTOURNAMENT_API UUTRatingManager : public UObject
{
    GENERATED_BODY()

public:
    UUTRatingManager();

    /**
     * Process a completed ROUND (not match) and update all player ratings
     * Called after each elimination round in Team Showdown
     * @param GameState The game state containing round results
     * @param WinningTeam The team that won the round (nullptr for draw)
     */
    UFUNCTION(BlueprintCallable, Category="Rating")
    void ProcessRoundResult(class AUTGameState* GameState, class AUTTeamInfo* WinningTeam);

    /**
     * Get a player's current rating
     * @param UniqueId Player's unique ID
     * @param OutRating Current rating (R)
     * @param OutRD Current rating deviation
     * @param OutVolatility Current volatility
     * @return True if player rating found
     */
    UFUNCTION(BlueprintCallable, Category="Rating")
    bool GetPlayerRating(const FUniqueNetIdRepl& UniqueId, float& OutRating, float& OutRD, float& OutVolatility);

    /**
     * Set a player's rating (for initialization or manual adjustment)
     */
    UFUNCTION(BlueprintCallable, Category="Rating")
    void SetPlayerRating(const FUniqueNetIdRepl& UniqueId, float Rating, float RD, float Volatility);

protected:
    /** Map of player unique IDs to their Glicko-2 ratings */
    TMap<FString, TeamGlicko2::PlayerRating> PlayerRatings;

    /** Compute performance score for a player based on their stats */
    double ComputePerformanceScore(class AUTPlayerState* PlayerState);
    
    /** Save ratings to persistent storage (MCP profile or local file) */
    void SaveRatings();
    
    /** Load ratings from persistent storage */
    void LoadRatings();
};
```

**File: `UnrealTournament/Source/UnrealTournament/Private/UTRatingManager.cpp`**

```cpp
#include "UnrealTournament.h"
#include "UTRatingManager.h"
#include "UTGameState.h"
#include "UTPlayerState.h"
#include "UTTeamInfo.h"
#include "TeamGlicko2System.h"

UUTRatingManager::UUTRatingManager()
{
    // Load existing ratings on construction
    LoadRatings();
}

void UUTRatingManager::ProcessRoundResult(AUTGameState* GameState, AUTTeamInfo* WinningTeam)
{
    if (!GameState || !GameState->Teams.Num())
    {
        return; // Not a team game or invalid state
    }

    // Create TeamGlicko2 match result structure
    TeamGlicko2::MatchResult Match;

    // Determine match scores (1.0 = win, 0.0 = loss, 0.5 = draw)
    if (WinningTeam == nullptr)
    {
        // Draw
        Match.scoreA = TeamGlicko2::kDrawScore;
        Match.scoreB = TeamGlicko2::kDrawScore;
    }
    else
    {
        // One team won
        Match.scoreA = (WinningTeam == GameState->Teams[0]) ? TeamGlicko2::kWinScore : TeamGlicko2::kLossScore;
        Match.scoreB = (WinningTeam == GameState->Teams[1]) ? TeamGlicko2::kWinScore : TeamGlicko2::kLossScore;
    }

    // Process each team
    for (int32 TeamIndex = 0; TeamIndex < 2 && TeamIndex < GameState->Teams.Num(); TeamIndex++)
    {
        AUTTeamInfo* TeamInfo = GameState->Teams[TeamIndex];
        if (!TeamInfo) continue;

        // Get all players on this team
        for (APlayerState* PS : GameState->PlayerArray)
        {
            AUTPlayerState* UTPS = Cast<AUTPlayerState>(PS);
            if (UTPS && UTPS->Team == TeamInfo && !UTPS->bOnlySpectator)
            {
                // Get player's unique ID as string
                FString PlayerID = UTPS->UniqueId.ToString();
                
                // Get or create player rating
                TeamGlicko2::PlayerRating* Rating = PlayerRatings.Find(PlayerID);
                if (!Rating)
                {
                    // New player - initialize with defaults
                    PlayerRatings.Add(PlayerID, TeamGlicko2::PlayerRating());
                    Rating = PlayerRatings.Find(PlayerID);
                }

                // Compute performance score
                double PerformanceScore = ComputePerformanceScore(UTPS);

                // Add to appropriate team
                TeamGlicko2::MatchPlayer MatchPlayer(*Rating, PerformanceScore);
                if (TeamIndex == 0)
                {
                    Match.teamA.Add(MatchPlayer);
                }
                else
                {
                    Match.teamB.Add(MatchPlayer);
                }
            }
        }
    }

    // Process the match through Glicko-2 system
    TeamGlicko2::TeamGlicko2System::ProcessMatch(Match);

    // Update stored ratings with new values
    int32 PlayerIndex = 0;
    for (int32 TeamIndex = 0; TeamIndex < 2 && TeamIndex < GameState->Teams.Num(); TeamIndex++)
    {
        AUTTeamInfo* TeamInfo = GameState->Teams[TeamIndex];
        if (!TeamInfo) continue;

        for (APlayerState* PS : GameState->PlayerArray)
        {
            AUTPlayerState* UTPS = Cast<AUTPlayerState>(PS);
            if (UTPS && UTPS->Team == TeamInfo && !UTPS->bOnlySpectator)
            {
                FString PlayerID = UTPS->UniqueId.ToString();
                
                // Get updated rating from match result
                const TeamGlicko2::MatchPlayer& UpdatedPlayer = 
                    (TeamIndex == 0) ? Match.teamA[PlayerIndex] : Match.teamB[PlayerIndex];
                
                // Store updated rating
                PlayerRatings[PlayerID] = UpdatedPlayer.rating;
                
                PlayerIndex++;
            }
        }
    }

    // Save to persistent storage
    SaveRatings();
}

double UUTRatingManager::ComputePerformanceScore(AUTPlayerState* PlayerState)
{
    if (!PlayerState)
    {
        return 0.0;
    }

    // **IMPORTANT: Use ROUND stats, not match aggregate stats**
    // For Team Showdown, use RoundKills, RoundDeaths, RoundDamageDone
    
    double Score = 0.0;
    
    // Round-specific stats (resets each round in Showdown)
    Score += 250.0 * PlayerState->RoundKills;        // Heavy weight on kills
    Score -= 100.0 * PlayerState->RoundDeaths;       // Penalty for deaths
    Score += 0.5 * PlayerState->RoundDamageDone;     // Damage contribution
    
    // Optional: Add bonuses for special achievements
    // - First blood: +100
    // - Multi-kill: +50 per extra kill
    // - Survival (didn't die): +50
    
    // Ensure minimum score (avoid negative/zero performance)
    return FMath::Max(100.0, Score);
}

bool UUTRatingManager::GetPlayerRating(const FUniqueNetIdRepl& UniqueId, float& OutRating, float& OutRD, float& OutVolatility)
{
    FString PlayerID = UniqueId.ToString();
    TeamGlicko2::PlayerRating* Rating = PlayerRatings.Find(PlayerID);
    
    if (Rating)
    {
        OutRating = Rating->GetRating();
        OutRD = Rating->GetRD();
        OutVolatility = Rating->GetVolatility();
        return true;
    }
    
    return false;
}

void UUTRatingManager::SetPlayerRating(const FUniqueNetIdRepl& UniqueId, float Rating, float RD, float Volatility)
{
    FString PlayerID = UniqueId.ToString();
    PlayerRatings.Add(PlayerID, TeamGlicko2::PlayerRating(Rating, RD, Volatility));
    SaveRatings();
}

void UUTRatingManager::SaveRatings()
{
    // TODO: Implement persistence
    // Options:
    // 1. Save to MCP profile system (if using Epic backend)
    // 2. Save to local JSON file on server
    // 3. Save to database
    
    UE_LOG(LogUTGame, Log, TEXT("Saving %d player ratings"), PlayerRatings.Num());
}

void UUTRatingManager::LoadRatings()
{
    // TODO: Implement loading from persistence
    UE_LOG(LogUTGame, Log, TEXT("Loading player ratings"));
}
```

#### Step 3: Hook into Team Showdown Round End

**File: `UnrealTournament/Source/UnrealTournament/Private/UTTeamShowdownGame.cpp`**

Modify the `StartIntermission()` function to process ratings after each round:

```cpp
#include "UTRatingManager.h"

void AUTTeamShowdownGame::StartIntermission()
{
    ClearTimerUFunc(this, FName(TEXT("StartIntermission")));
    
    // **EXISTING CODE: Award survival bonus**
    if (LastRoundWinner != NULL)
    {
        for (AController* C : LastRoundWinner->GetTeamMembers())
        {
            AUTPlayerState* PS = Cast<AUTPlayerState>(C->PlayerState);
            if (PS != NULL)
            {
                PS->AdjustScore(50);
            }
        }
    }
    
    // **NEW: Process round ratings**
    // Get or create rating manager
    if (!RatingManager)
    {
        RatingManager = NewObject<UUTRatingManager>(this);
    }
    
    if (RatingManager && UTGameState)
    {
        // Determine if round was a draw
        bool bWasDraw = (LastRoundWinner == nullptr);
        
        // Process the round result
        RatingManager->ProcessRoundResult(UTGameState, LastRoundWinner);
        
        // Optional: Log rating changes
        UE_LOG(LogUTGame, Log, TEXT("Processed round ratings. Winner: %s"), 
            bWasDraw ? TEXT("DRAW") : *LastRoundWinner->TeamName.ToString());
    }
    
    // **EXISTING CODE: Continue with intermission**
    bPastELOLimit = true;
    
    // Process ratings if this was a ranked/competitive match
    if (ShouldProcessRatings())
    {
        UUTRatingManager* RatingManager = NewObject<UUTRatingManager>();
        if (RatingManager && UTGameState)
        {
            // Determine winning team
            AUTTeamInfo* WinningTeam = nullptr;
            if (UTGameState->WinningTeam)
            {
                WinningTeam = UTGameState->WinningTeam;
            }
            // else nullptr = draw
            
            RatingManager->ProcessMatchResult(UTGameState, WinningTeam);
            
            UE_LOG(LogUTGame, Log, TEXT("Processed match ratings"));
        }
    }
}

bool AUTTeamGameMode::ShouldProcessRatings() const
{
    // Only process ratings for:
    // - Ranked matches
    // - Matches that completed normally (not aborted)
    // - Matches with minimum player count
    // - Matches that lasted minimum duration
    
    return !bGameEnded &&  // Match completed normally
           UTGameState &&
           UTGameState->bRankedSession &&  // Ranked match
           UTGameState->PlayerArray.Num() >= MinPlayersForRating &&
           GetWorld()->TimeSeconds > MinMatchTimeForRating;
}
```

### Option 2: Plugin Architecture

Create a standalone plugin that can be enabled/disabled per server.

**File: `UnrealTournament/Plugins/UTRatingSystem/UTRatingSystem.uplugin`**

```json
{
    "FileVersion": 3,
    "Version": 1,
    "VersionName": "1.0",
    "FriendlyName": "UT Rating System",
    "Description": "Team-based Glicko-2 rating system for Unreal Tournament",
    "Category": "UnrealTournament.Gameplay",
    "CreatedBy": "Community",
    "CreatedByURL": "",
    "DocsURL": "",
    "MarketplaceURL": "",
    "SupportURL": "",
    "CanContainContent": false,
    "IsBetaVersion": false,
    "Installed": false,
    "Modules": [
        {
            "Name": "UTRatingSystem",
            "Type": "Runtime",
            "LoadingPhase": "Default"
        }
    ]
}
```

## Performance Score Customization

Different game modes should weight stats differently. Here are recommendations:

### Team Deathmatch (TDM)
```cpp
double Score = 1.0 * Kills 
             - 0.5 * Deaths 
             + 0.001 * DamageDone;
```

### Capture the Flag (CTF)
```cpp
double Score = 1.0 * Kills
             - 0.5 * Deaths
             + 10.0 * FlagCaptures
             + 5.0 * FlagReturns
             + 2.0 * FlagAssists
             + 0.001 * DamageDone;
```

### Domination
```cpp
double Score = 1.0 * Kills
             - 0.5 * Deaths
             + 3.0 * ControlPointCaptures
             + 0.5 * ControlPointDefenses
             + 0.001 * DamageDone;
```

## Persistence Options

### 1. MCP Profile System (Epic Backend)
```cpp
void UUTRatingManager::SaveToMCP(AUTPlayerState* PlayerState)
{
    // Use Epic's MCP profile system
    #if WITH_PROFILE
    if (PlayerState->McpProfile)
    {
        PlayerState->McpProfile->SetStat(
            "ElimRank_Rating", 
            FString::SanitizeFloat(Rating)
        );
    }
    #endif
}
```

### 2. Local JSON File (Server-Side)
```cpp
void UUTRatingManager::SaveToFile()
{
    FString SavePath = FPaths::ProjectSavedDir() / TEXT("Ratings/PlayerRatings.json");
    
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    
    for (auto& Pair : PlayerRatings)
    {
        TSharedPtr<FJsonObject> PlayerObject = MakeShareable(new FJsonObject);
        PlayerObject->SetNumberField("Rating", Pair.Value.GetRating());
        PlayerObject->SetNumberField("RD", Pair.Value.GetRD());
        PlayerObject->SetNumberField("Volatility", Pair.Value.GetVolatility());
        
        RootObject->SetObjectField(Pair.Key, PlayerObject);
    }
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    
    FFileHelper::SaveStringToFile(OutputString, *SavePath);
}
```

### 3. Database Integration
```cpp
// Use sqlite3 (already in UT codebase at ThirdParty/sqlite/)
// or connect to external MySQL/PostgreSQL server
```

## Configuration

Add config variables to control the system:

**File: `UnrealTournament/Config/DefaultGame.ini`**

```ini
[/Script/UnrealTournament.UTTeamGameMode]
bEnableRatingSystem=true
MinPlayersForRating=8
MinMatchTimeForRating=300.0
bOnlyRankedMatches=true

[/Script/UnrealTournament.UTRatingManager]
PerformanceAlpha=0.2
PerformanceWeightMin=0.5
PerformanceWeightMax=1.5
EnableRatingClamp=true
MaxRatingChangePerMatch=300.0
```

## Testing

1. **Unit Tests**: Test rating calculations with known inputs
2. **Integration Tests**: Test full match processing
3. **Balance Testing**: Monitor rating distribution over time
4. **Performance Testing**: Ensure no frame drops during processing

## Matchmaking Integration

Once ratings are tracked, use them for matchmaking:

```cpp
bool AUTLobbyGameMode::ShouldJoinInstance(AUTPlayerState* Player, AUTLobbyMatchInfo* Match)
{
    // Get player rating
    float PlayerRating, PlayerRD, PlayerVol;
    if (RatingManager->GetPlayerRating(Player->UniqueId, PlayerRating, PlayerRD, PlayerVol))
    {
        // Get average rating of match
        float MatchAvgRating = Match->GetAverageRating();
        
        // Only allow join if within ±200 rating points
        if (FMath::Abs(PlayerRating - MatchAvgRating) > 200.0f)
        {
            return false;
        }
    }
    
    return true;
}
```

## Display Integration

Show ratings in scoreboard, player cards, and menus:

```cpp
// In UTScoreboard or HUD
FText RatingText = FText::Format(
    NSLOCTEXT("Rating", "PlayerRating", "Rating: {0} (±{1})"),
    FText::AsNumber((int32)Rating),
    FText::AsNumber((int32)RD)
);
```

## Future Enhancements

1. **Decay System**: Implement rating decay for inactive players
2. **Placement Matches**: New players play 10 placement matches with higher RD
3. **Seasonal Resets**: Reset ratings periodically with soft reset
4. **Leaderboards**: Display top-rated players
5. **Ranked Tiers**: Bronze, Silver, Gold, Platinum, Diamond, etc.
6. **Team MMR**: Track team compositions and synergy

## References

- TeamGlicko2 Implementation: `ElimRank/README.md`
- Mathematical Specification: `ElimRank/docs/implementation.latex`
- UT Game Mode Documentation: `UnrealTournament/Source/UnrealTournament/Public/UTGameMode.h`
