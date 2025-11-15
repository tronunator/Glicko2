#include <iostream>
#include <vector>
#include <iomanip>
#include "TeamGlicko2System.h"

using namespace TeamGlicko2;

// Helper function to compute a sample performance score
// In a real game, this would use actual game statistics
double ComputePerformanceScore(int kills, int deaths, double damage, double objectiveScore) {
    return TeamGlicko2::kKillWeight * kills
         + TeamGlicko2::kDeathWeight * deaths
         + TeamGlicko2::kDamageWeight * damage
         + TeamGlicko2::kObjectiveWeight * objectiveScore;
}

// Print player rating details
void PrintPlayer(const std::string& name, const PlayerRating& rating) {
    std::cout << std::setw(15) << name << ": "
              << "Rating = " << std::setw(7) << std::fixed << std::setprecision(2) << rating.GetRating()
              << ", RD = " << std::setw(6) << std::fixed << std::setprecision(2) << rating.GetRD()
              << ", Volatility = " << std::fixed << std::setprecision(4) << rating.GetVolatility()
              << std::endl;
}

// Print team statistics
void PrintTeamStats(const std::string& teamName, const std::vector<MatchPlayer>& team) {
    std::cout << "\n" << teamName << ":" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (size_t i = 0; i < team.size(); ++i) {
        std::string playerName = "Player " + std::to_string(i + 1);
        PrintPlayer(playerName, team[i].rating);
        std::cout << "    Performance Score: " << team[i].performanceScore << std::endl;
    }
}

// Example 1: Basic 4v4 match with balanced teams
void Example_Balanced4v4() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "EXAMPLE 1: Balanced 4v4 Match - Team A Wins" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    MatchResult match;

    // Team A: All players start at 1500 rating
    match.teamA.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(25, 10, 3500, 5)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(20, 12, 3200, 4)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(18, 15, 2800, 3)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(15, 18, 2500, 2)));

    // Team B: All players start at 1500 rating
    match.teamB.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(22, 15, 3100, 3)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(18, 17, 2900, 3)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(15, 20, 2600, 2)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(12, 22, 2200, 2)));

    // Team A wins
    match.scoreA = TeamGlicko2::kWinScore;
    match.scoreB = TeamGlicko2::kLossScore;

    std::cout << "\nBEFORE MATCH:" << std::endl;
    PrintTeamStats("Team A (Winner)", match.teamA);
    PrintTeamStats("Team B (Loser)", match.teamB);
    // Process the match
    TeamGlicko2System::ProcessMatch(match);
    std::cout << "\nAFTER MATCH:" << std::endl;
    PrintTeamStats("Team A (Winner)", match.teamA);
    PrintTeamStats("Team B (Loser)", match.teamB);
    std::cout << "\nNOTES:" << std::endl;
    std::cout << "- Team A's best performer (Player 1) gained the most rating" << std::endl;
    std::cout << "- Team A's worst performer (Player 4) gained the least rating" << std::endl;
    std::cout << "- Team B's best performer (Player 1) lost the least rating" << std::endl;
    std::cout << "- Team B's worst performer (Player 4) lost the most rating" << std::endl;
}

// Example 2: Upset victory - lower rated team wins
void Example_UpsetVictory() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "EXAMPLE 2: Upset Victory - Lower Rated Team Wins" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    MatchResult match;
    // Team A: Lower rated team (average ~1400)
    match.teamA.push_back(MatchPlayer(PlayerRating(1420, 180), ComputePerformanceScore(28, 8, 4000, 6)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1390, 190), ComputePerformanceScore(24, 10, 3600, 5)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1380, 200), ComputePerformanceScore(20, 12, 3200, 4)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1410, 185), ComputePerformanceScore(18, 14, 2900, 3)));
    // Team B: Higher rated team (average ~1600)
    match.teamB.push_back(MatchPlayer(PlayerRating(1620, 150), ComputePerformanceScore(20, 18, 3400, 4)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1590, 160), ComputePerformanceScore(18, 20, 3100, 3)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1580, 165), ComputePerformanceScore(15, 22, 2800, 2)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1610, 155), ComputePerformanceScore(12, 24, 2400, 2)));
    // Team A wins (upset)
    match.scoreA = TeamGlicko2::kWinScore;
    match.scoreB = TeamGlicko2::kLossScore;

    std::cout << "\nBEFORE MATCH:" << std::endl;
    PrintTeamStats("Team A (Underdog Winner)", match.teamA);
    PrintTeamStats("Team B (Favorite Loser)", match.teamB);

    // Process the match
    TeamGlicko2System::ProcessMatch(match);
    std::cout << "\nAFTER MATCH:" << std::endl;
    PrintTeamStats("Team A (Underdog Winner)", match.teamA);
    PrintTeamStats("Team B (Favorite Loser)", match.teamB);
    std::cout << "\nNOTES:" << std::endl;
    std::cout << "- Team A gains significant rating for upset victory" << std::endl;
    std::cout << "- Team B loses significant rating for unexpected loss" << std::endl;
    std::cout << "- Performance weighting still applies within each team" << std::endl;
}

// Example 3: 5v5 match with extreme performance differences
void Example_ExtremeDifference5v5() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "EXAMPLE 3: 5v5 Match with Extreme Performance Differences" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    MatchResult match;
    // Team A: One player hard carries
    match.teamA.push_back(MatchPlayer(PlayerRating(1550, 180), ComputePerformanceScore(40, 5, 6000, 10)));  // Carry
    match.teamA.push_back(MatchPlayer(PlayerRating(1520, 190), ComputePerformanceScore(15, 15, 2500, 3)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1530, 185), ComputePerformanceScore(12, 18, 2200, 2)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1510, 200), ComputePerformanceScore(10, 20, 1800, 2)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1540, 175), ComputePerformanceScore(8, 22, 1500, 1)));   // Feeding
    // Team B: Balanced performance but lost
    match.teamB.push_back(MatchPlayer(PlayerRating(1560, 170), ComputePerformanceScore(20, 15, 3200, 4)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1570, 165), ComputePerformanceScore(19, 16, 3100, 4)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1580, 160), ComputePerformanceScore(18, 17, 3000, 3)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1550, 175), ComputePerformanceScore(17, 18, 2900, 3)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1540, 180), ComputePerformanceScore(16, 19, 2800, 3)));
    // Team A wins
    match.scoreA = TeamGlicko2::kWinScore;
    match.scoreB = TeamGlicko2::kLossScore;
    std::cout << "\nBEFORE MATCH:" << std::endl;
    PrintTeamStats("Team A (Winner, One Carry)", match.teamA);
    PrintTeamStats("Team B (Loser, Balanced)", match.teamB);
    // Process the match
    TeamGlicko2System::ProcessMatch(match);
    std::cout << "\nAFTER MATCH:" << std::endl;
    PrintTeamStats("Team A (Winner, One Carry)", match.teamA);
    PrintTeamStats("Team B (Loser, Balanced)", match.teamB);
    std::cout << "\nNOTES:" << std::endl;
    std::cout << "- Team A Player 1 (carry) gains massive rating boost" << std::endl;
    std::cout << "- Team A Player 5 (feeder) gains minimal rating despite winning" << std::endl;
    std::cout << "- Team B has uniform rating losses due to similar performance" << std::endl;
}

// Example 4: Draw scenario
void Example_DrawMatch() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "EXAMPLE 4: 4v4 Draw Match" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    MatchResult match;
    // Team A
    match.teamA.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(20, 12, 3200, 4)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(18, 14, 3000, 3)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(16, 16, 2800, 3)));
    match.teamA.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(14, 18, 2600, 2)));
    // Team B
    match.teamB.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(19, 13, 3100, 4)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(17, 15, 2950, 3)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(15, 17, 2750, 3)));
    match.teamB.push_back(MatchPlayer(PlayerRating(1500, 200), ComputePerformanceScore(13, 19, 2550, 2)));
    // Draw
    match.scoreA = TeamGlicko2::kDrawScore;
    match.scoreB = TeamGlicko2::kDrawScore;
    std::cout << "\nBEFORE MATCH:" << std::endl;
    PrintTeamStats("Team A", match.teamA);
    PrintTeamStats("Team B", match.teamB);
    // Process the match
    TeamGlicko2System::ProcessMatch(match);
    std::cout << "\nAFTER MATCH:" << std::endl;
    PrintTeamStats("Team A", match.teamA);
    PrintTeamStats("Team B", match.teamB);
    std::cout << "\nNOTES:" << std::endl;
    std::cout << "- Ratings change slightly based on performance weighting" << std::endl;
    std::cout << "- Top performers gain slight rating, bottom lose slight rating" << std::endl;
    std::cout << "- RD decreases for all players due to match participation" << std::endl;
}

int main() {
    std::cout << std::fixed << std::setprecision(2);

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║       Team-Based Glicko-2 Rating System - Example Usage           ║\n";
    std::cout << "║    Performance-Weighted Rating Updates for NvsN Team Matches      ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";

    // Run all examples
    Example_Balanced4v4();
    Example_UpsetVictory();
    Example_ExtremeDifference5v5();
    Example_DrawMatch();

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "All examples completed successfully!" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    std::cout << "\nKEY FEATURES DEMONSTRATED:" << std::endl;
    std::cout << "1. Win/loss is the primary driver of rating changes" << std::endl;
    std::cout << "2. Performance weighting modulates rating change magnitude" << std::endl;
    std::cout << "3. Top performers gain more (or lose less) rating" << std::endl;
    std::cout << "4. Bottom performers gain less (or lose more) rating" << std::endl;
    std::cout << "5. Upset victories result in larger rating swings" << std::endl;
    std::cout << "6. Rating deviation (uncertainty) decreases with matches" << std::endl;
    std::cout << "7. Team average weight always equals 1.0 (normalized)" << std::endl;

    return 0;
}
