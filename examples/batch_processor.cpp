// batch_processor.cpp - Process match CSV data through TeamGlicko2 system
// Reads test/match_stats.csv and outputs rating evolution to rating_results.csv

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "TeamGlicko2System.h"

using namespace TeamGlicko2;

// CSV parsing helper
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Match data structure
struct PlayerMatchData {
    std::string playerId;
    std::string playerName;
    std::string team;  // "Red" or "Blue"
    int kills;
    int deaths;
    double damage;
    double score;
};

struct MatchData {
    int matchId;
    std::vector<PlayerMatchData> players;
    std::string winner;  // "Red", "Blue", or "Draw"
};

// Compute performance score from stats
double ComputePerformanceScore(const PlayerMatchData& player) {
    // Use config constants for tunable formula
    double perfScore = (player.kills * kKillWeight) 
                     + (player.deaths * kDeathWeight)  // Note: kDeathWeight is negative
                     + (player.damage * kDamageWeight)
                     + (player.score * kObjectiveWeight);
    
    return std::max(100.0, perfScore);  // Minimum 100
}

int main(int argc, char* argv[]) {
    std::string inputFile = "test/match_stats.csv";
    std::string outputFile = "test/rating_results.csv";
    
    if (argc > 1) inputFile = argv[1];
    if (argc > 2) outputFile = argv[2];
    
    std::cout << "TeamGlicko2 Batch Processor" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << "Input: " << inputFile << std::endl;
    std::cout << "Output: " << outputFile << std::endl << std::endl;
    
    // Read match stats
    std::ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        std::cerr << "Error: Cannot open " << inputFile << std::endl;
        return 1;
    }
    
    // Parse CSV header
    std::string line;
    std::getline(inFile, line);
    std::vector<std::string> headers = split(line, ',');
    
    // Find column indices
    int scoreIdx = -1, killsIdx = -1, deathIdx = -1, teamIdx = -1;
    int damageIdx = -1, matchIdIdx = -1, playerIdIdx = -1, winnerIdx = -1, playerNameIdx = -1;
    
    for (size_t i = 0; i < headers.size(); i++) {
        if (headers[i] == "Score") scoreIdx = i;
        else if (headers[i] == "KILLS") killsIdx = i;
        else if (headers[i] == "DEATH") deathIdx = i;
        else if (headers[i] == "TEAM") teamIdx = i;
        else if (headers[i] == "DAMAGE") damageIdx = i;
        else if (headers[i] == "MATCHID") matchIdIdx = i;
        else if (headers[i] == "PLAYERID") playerIdIdx = i;
        else if (headers[i] == "PlayerName") playerNameIdx = i;
        else if (headers[i] == "Winner") winnerIdx = i;
    }
    
    if (killsIdx == -1 || deathIdx == -1 || teamIdx == -1 || 
        matchIdIdx == -1 || playerIdIdx == -1 || winnerIdx == -1 || playerNameIdx == -1) {
        std::cerr << "Error: Missing required columns (need: KILLS, DEATH, TEAM, MATCHID, PLAYERID, PlayerName, Winner)" << std::endl;
        return 1;
    }
    
    // Group by match
    std::map<int, MatchData> matches;
    while (std::getline(inFile, line)) {
        auto tokens = split(line, ',');
        if (tokens.size() < headers.size()) continue;
        
        std::string team = tokens[teamIdx];
        if (team != "Red" && team != "Blue") continue;  // Skip spectators
        
        int matchId = std::stoi(tokens[matchIdIdx]);
        
        PlayerMatchData pmd;
        pmd.playerId = tokens[playerIdIdx];
        pmd.playerName = tokens[playerNameIdx];
        pmd.team = team;
        pmd.kills = std::stoi(tokens[killsIdx]);
        pmd.deaths = std::stoi(tokens[deathIdx]);
        pmd.damage = (damageIdx >= 0) ? std::stod(tokens[damageIdx]) : 0.0;
        pmd.score = (scoreIdx >= 0) ? std::stod(tokens[scoreIdx]) : 0.0;
        
        matches[matchId].matchId = matchId;
        matches[matchId].players.push_back(pmd);
        matches[matchId].winner = tokens[winnerIdx];  // Winner is in the CSV now
    }
    inFile.close();
    
    std::cout << "Loaded " << matches.size() << " matches" << std::endl << std::endl;
    
    // Store player ratings
    std::map<std::string, PlayerRating> playerRatings;
    
    // Open output file
    std::ofstream outFile(outputFile);
    outFile << "MatchID,PlayerID,PlayerName,Team,Kills,Deaths,Damage,PerformanceScore,"
            << "RatingBefore,RDBefore,RatingAfter,RDAfter,RatingChange" << std::endl;
    
    // Process matches in order
    int processedCount = 0;
    for (auto& matchPair : matches) {
        MatchData& match = matchPair.second;
        
        // Build MatchResult
        MatchResult result;
        
        // Set scores
        if (match.winner == "Draw") {
            result.scoreA = kDrawScore;
            result.scoreB = kDrawScore;
        } else if (match.winner == "Red") {
            result.scoreA = kWinScore;
            result.scoreB = kLossScore;
        } else if (match.winner == "Blue") {
            result.scoreA = kLossScore;
            result.scoreB = kWinScore;
        } else {
            std::cerr << "Warning: Unknown winner for match " << match.matchId << std::endl;
            continue;
        }
        
        // Get or create ratings for all players
        for (const auto& player : match.players) {
            if (playerRatings.find(player.playerId) == playerRatings.end()) {
                playerRatings[player.playerId] = PlayerRating();
            }
        }
        
        // Build team A (Red) and team B (Blue)
        for (const auto& player : match.players) {
            PlayerRating ratingBefore = playerRatings[player.playerId];
            double perfScore = ComputePerformanceScore(player);
            
            MatchPlayer mp(ratingBefore, perfScore);
            
            if (player.team == "Red") {
                result.teamA.push_back(mp);
            } else {
                result.teamB.push_back(mp);
            }
        }
        
        // Skip if teams are invalid
        if (result.teamA.empty() || result.teamB.empty()) {
            continue;
        }
        
        // Process match
        TeamGlicko2System::ProcessMatch(result);
        
        // Update stored ratings and write output
        size_t redIdx = 0, blueIdx = 0;
        for (const auto& player : match.players) {
            PlayerRating ratingBefore = playerRatings[player.playerId];
            
            PlayerRating ratingAfter;
            if (player.team == "Red" && redIdx < result.teamA.size()) {
                ratingAfter = result.teamA[redIdx++].rating;
            } else if (player.team == "Blue" && blueIdx < result.teamB.size()) {
                ratingAfter = result.teamB[blueIdx++].rating;
            } else {
                continue;
            }
            
            playerRatings[player.playerId] = ratingAfter;
            
            double perfScore = ComputePerformanceScore(player);
            double ratingChange = ratingAfter.GetRating() - ratingBefore.GetRating();
            
            outFile << match.matchId << ","
                    << player.playerId << ","
                    << player.playerName << ","
                    << player.team << ","
                    << player.kills << ","
                    << player.deaths << ","
                    << player.damage << ","
                    << perfScore << ","
                    << ratingBefore.GetRating() << ","
                    << ratingBefore.GetRD() << ","
                    << ratingAfter.GetRating() << ","
                    << ratingAfter.GetRD() << ","
                    << ratingChange << std::endl;
        }
        
        processedCount++;
        if (processedCount % 100 == 0) {
            std::cout << "Processed " << processedCount << " matches..." << std::endl;
        }
    }
    
    outFile.close();
    
    std::cout << std::endl << "Summary:" << std::endl;
    std::cout << "========" << std::endl;
    std::cout << "Processed: " << processedCount << " matches" << std::endl;
    std::cout << "Unique players: " << playerRatings.size() << std::endl;
    std::cout << "Output written to: " << outputFile << std::endl;
    
    // Show top 10 players by rating
    std::vector<std::pair<std::string, PlayerRating>> sortedPlayers(
        playerRatings.begin(), playerRatings.end()
    );
    std::sort(sortedPlayers.begin(), sortedPlayers.end(),
              [](const auto& a, const auto& b) {
                  return a.second.GetRating() > b.second.GetRating();
              });
    
    std::cout << std::endl << "Top 10 Players:" << std::endl;
    std::cout << "===============" << std::endl;
    for (size_t i = 0; i < std::min(size_t(10), sortedPlayers.size()); i++) {
        std::cout << (i+1) << ". " << sortedPlayers[i].first.substr(0, 8) << "... "
                  << "Rating: " << sortedPlayers[i].second.GetRating() 
                  << " (RD: " << sortedPlayers[i].second.GetRD() << ")"
                  << std::endl;
    }
    
    return 0;
}
