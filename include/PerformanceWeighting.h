#ifndef PERFORMANCE_WEIGHTING_H
#define PERFORMANCE_WEIGHTING_H

#include <vector>
#include <cmath>
#include "TeamGlicko2Config.h"

namespace TeamGlicko2
{
    /// Represents performance-based weight for a single player
    struct PlayerWeight
    {
        int playerIndex;        // Index in team array
        double performanceScore; // Raw performance score p_i
        double zScore;          // Standardized z-score
        double rawWeight;       // w_i^raw before clamping
        double clampedWeight;   // w_i after clamping
        double normalizedWeight; // Final normalized weight w_i_tilde
        
        PlayerWeight() 
            : playerIndex(0), performanceScore(0.0), zScore(0.0), 
              rawWeight(1.0), clampedWeight(1.0), normalizedWeight(1.0) {}
    };
    
    /// Computes performance-based weights for team members
    /// Implements the algorithm from Section 5 of the specification
    class PerformanceWeighting
    {
    public:
        /// Compute normalized performance weights for all players in a team
        /// @param performanceScores Raw performance scores for each player
        /// @param alpha Sensitivity parameter (default from config)
        /// @param wMin Minimum weight (default from config)
        /// @param wMax Maximum weight (default from config)
        /// @return Vector of PlayerWeight structs with all computed values
        static std::vector<PlayerWeight> ComputeWeights(
            const std::vector<double>& performanceScores,
            double alpha = TeamGlicko2::kAlpha,
            double wMin = TeamGlicko2::kWeightMin,
            double wMax = TeamGlicko2::kWeightMax);
        
        /// Compute mean performance score for a team
        static double ComputeMean(const std::vector<double>& scores);
        
        /// Compute standard deviation of performance scores
        /// Includes epsilon to avoid division by zero
        static double ComputeStdDev(const std::vector<double>& scores, double mean);
        
        /// Compute z-score for a performance value
        /// z_i = (p_i - mean) / stddev
        static double ComputeZScore(double score, double mean, double stddev);
        
        /// Compute raw weight from z-score
        /// w_i^raw = 1 + alpha * z_i
        static double ComputeRawWeight(double zScore, double alpha);
        
        /// Clamp weight to valid range [wMin, wMax]
        static double ClampWeight(double weight, double wMin, double wMax);
        
        /// Normalize weights so their average equals 1.0
        /// w_i_tilde = w_i * |T| / sum(w_j)
        static void NormalizeWeights(std::vector<PlayerWeight>& weights);
    };
    
} // namespace TeamGlicko2

#endif // PERFORMANCE_WEIGHTING_H
