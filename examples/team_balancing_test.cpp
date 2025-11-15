#include <iostream>
#include <iomanip>
#include "TeamBalancer.h"

using namespace TeamGlicko2;

BalancerConfig config;

// Helper function to print player info
void PrintPlayer(int id, const PlayerInfo& player) {
    std::cout << "  Player " << id << ": "
              << "Rating=" << std::fixed << std::setprecision(0) << player.rating.GetRating()
              << ", RD=" << std::setprecision(0) << player.rating.GetRD()
              << ", Eff=" << std::setprecision(1) << player.effectiveRating
              << std::endl;
}

// Helper function to print team assignment
void PrintTeamAssignment(const std::vector<PlayerInfo>& players, const TeamAssignment& assignment) {
    std::cout << "\n" << std::string(70, '-') << std::endl;
    std::cout << "TEAM ASSIGNMENT RESULT" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    std::cout << "\nTeam 0 (Strength: " << std::fixed << std::setprecision(1)
              << assignment.team0Strength << ", Uncertainty: " << assignment.team0Uncertainty << "):" << std::endl;
    for (int playerId : assignment.team0PlayerIds) {
        // Find player in original list
        for (const auto& p : players) {
            if (p.playerId == playerId) {
                PrintPlayer(playerId, p);
                break;
            }
        }
    }

    std::cout << "\nTeam 1 (Strength: " << std::fixed << std::setprecision(1)
              << assignment.team1Strength << ", Uncertainty: " << assignment.team1Uncertainty << "):" << std::endl;
    for (int playerId : assignment.team1PlayerIds) {
        // Find player in original list
        for (const auto& p : players) {
            if (p.playerId == playerId) {
                PrintPlayer(playerId, p);
                break;
            }
        }
    }

    std::cout << "\nObjective J(A,B): " << std::fixed << std::setprecision(2)
              << assignment.objectiveValue << std::endl;
    std::cout << "  Strength Difference: " << assignment.strengthDifference << std::endl;
    std::cout << "  Uncertainty Difference: " << assignment.uncertaintyDifference << std::endl;
    std::cout << "  Pure Rating Difference: " << assignment.pureRatingDifference << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}


// Test 1: Top player constraint
void Test_TopPlayerConstraint() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "TEST 1: Fair even lobby" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    std::vector<PlayerInfo> players;
    // Two dominant players
    players.push_back(PlayerInfo(1, PlayerRating(2200, 100)));  // Best
    players.push_back(PlayerInfo(2, PlayerRating(2150, 110)));  // Second best

    // Six average players
    players.push_back(PlayerInfo(3, PlayerRating(1500, 150)));
    players.push_back(PlayerInfo(4, PlayerRating(1490, 150)));
    players.push_back(PlayerInfo(5, PlayerRating(1480, 150)));
    players.push_back(PlayerInfo(6, PlayerRating(1470, 150)));
    players.push_back(PlayerInfo(7, PlayerRating(1460, 150)));
    players.push_back(PlayerInfo(8, PlayerRating(1450, 150)));

    std::cout << "\nInput Players:" << std::endl;
    for (const auto& p : players) {
        PrintPlayer(p.playerId, p);
    }

    TeamAssignment result1 = TeamBalancer::BalanceTeams(players, config);
    PrintTeamAssignment(players, result1);
}

// Test 2: Extreme skill gap - Top player with low-rated player
void Test_ExtremeSkillGap() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "TEST 2: Top Player (2400) + Low Player (1000)" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    std::vector<PlayerInfo> players;
    // One top player
    players.push_back(PlayerInfo(1, PlayerRating(2400, 80)));

    // Six mid-tier players
    players.push_back(PlayerInfo(3, PlayerRating(1600, 150)));
    players.push_back(PlayerInfo(4, PlayerRating(1550, 160)));
    players.push_back(PlayerInfo(5, PlayerRating(1500, 140)));
    players.push_back(PlayerInfo(6, PlayerRating(1450, 150)));
    players.push_back(PlayerInfo(7, PlayerRating(1400, 160)));
    players.push_back(PlayerInfo(8, PlayerRating(1350, 140)));

    // One very low player
    players.push_back(PlayerInfo(2, PlayerRating(1000, 250)));  // New/weak player
    std::cout << "\nInput Players:" << std::endl;
    for (const auto& p : players) {
        PrintPlayer(p.playerId, p);
    }

    TeamAssignment result = TeamBalancer::BalanceTeams(players, config);
    PrintTeamAssignment(players, result);

    std::cout << "\nNote: System should balance the elite player with the weak player." << std::endl;
}

// Test 3: Top player with multiple weak players
void Test_TopPlayerMultipleWeakPlayers() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "TEST 3: Top Player (2500) + Two Weak Players (800, 900)" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    std::vector<PlayerInfo> players;
    // One elite player
    players.push_back(PlayerInfo(1, PlayerRating(2500, 70)));

    // Five mid-tier players
    players.push_back(PlayerInfo(4, PlayerRating(1550, 150)));
    players.push_back(PlayerInfo(5, PlayerRating(1500, 140)));
    players.push_back(PlayerInfo(6, PlayerRating(1450, 160)));
    players.push_back(PlayerInfo(7, PlayerRating(1400, 150)));
    players.push_back(PlayerInfo(8, PlayerRating(1350, 140)));

    // Two very weak players
    players.push_back(PlayerInfo(2, PlayerRating(800, 300)));
    players.push_back(PlayerInfo(3, PlayerRating(900, 280)));
    std::cout << "\nInput Players:" << std::endl;
    for (const auto& p : players) {
        PrintPlayer(p.playerId, p);
    }

    TeamAssignment result = TeamBalancer::BalanceTeams(players, config);
    PrintTeamAssignment(players, result);

    std::cout << "\nNote: System should pair elite player with weak players for balance." << std::endl;
}

// Test 4: Uneven lobby (7 players - 4v3)
void Test_UnevenLobby7Players() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "TEST 4: Uneven Lobby (7 Players) - Top Player in Smaller Team" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    std::vector<PlayerInfo> players;
    players.push_back(PlayerInfo(1, PlayerRating(2100, 50)));
    players.push_back(PlayerInfo(2, PlayerRating(1800, 80)));
    players.push_back(PlayerInfo(3, PlayerRating(1400, 150)));
    players.push_back(PlayerInfo(4, PlayerRating(1450, 140)));
    players.push_back(PlayerInfo(5, PlayerRating(1200, 160)));
    players.push_back(PlayerInfo(6, PlayerRating(1300, 170)));
    players.push_back(PlayerInfo(7, PlayerRating(1000, 100)));

    std::cout << "\nInput Players:" << std::endl;
    for (const auto& p : players) {
        PrintPlayer(p.playerId, p);
    }

    TeamAssignment result = TeamBalancer::BalanceTeams(players, config);
    PrintTeamAssignment(players, result);

    std::cout << "\nNote: With 7 players (3v4), top player (2100) must be in the 3-player team." << std::endl;
    std::cout << "      This compensates for the numerical disadvantage." << std::endl;
}

// Test 4: Uneven lobby (7 players - 4v3)
void Test_UnevenLobby7PlayersNoConstraint() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "TEST 4: Uneven Lobby (7 Players) - Top Player NOT forced in Smaller Team" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    std::vector<PlayerInfo> players;
    players.push_back(PlayerInfo(1, PlayerRating(2100, 50)));
    players.push_back(PlayerInfo(2, PlayerRating(1800, 80)));
    players.push_back(PlayerInfo(3, PlayerRating(1400, 150)));
    players.push_back(PlayerInfo(4, PlayerRating(1450, 140)));
    players.push_back(PlayerInfo(5, PlayerRating(1200, 160)));
    players.push_back(PlayerInfo(6, PlayerRating(1300, 170)));
    players.push_back(PlayerInfo(7, PlayerRating(1000, 100)));

    std::cout << "\nInput Players:" << std::endl;
    for (const auto& p : players) {
        PrintPlayer(p.playerId, p);
    }

    BalancerConfig config1;
    config1.putTopPlayerInSmallerTeam = false;  // Enable uneven team rule

    TeamAssignment result = TeamBalancer::BalanceTeams(players, config1);
    PrintTeamAssignment(players, result);

    std::cout << "\nNote: With 7 players (3v4), top player (2100) must be in the 3-player team." << std::endl;
    std::cout << "      This compensates for the numerical disadvantage." << std::endl;
}


int main() {
    std::cout << "\n";
    std::cout << "=====================================================================\n";
    std::cout << "|           Team Balancing Algorithm - Test Suite                   |\n";
    std::cout << "=====================================================================\n";

    std::cout << "Using lambda = " << config.lambda  << std::endl;
    Test_TopPlayerConstraint();
    Test_ExtremeSkillGap();
    Test_TopPlayerMultipleWeakPlayers();
    Test_UnevenLobby7Players();
    Test_UnevenLobby7PlayersNoConstraint();

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "All tests completed!" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    return 0;
}
