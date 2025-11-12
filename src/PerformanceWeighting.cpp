#include "PerformanceWeighting.h"
#include <cmath>
#include <algorithm>

namespace TeamGlicko2
{
    std::vector<PlayerWeight> PerformanceWeighting::ComputeWeights(
        const std::vector<double>& performanceScores,
        double alpha,
        double wMin,
        double wMax)
    {
        std::vector<PlayerWeight> weights;
        int teamSize = performanceScores.size();
        
        if (teamSize == 0)
        {
            return weights;
        }
        
        // Initialize weights array
        weights.resize(teamSize);
        for (int i = 0; i < teamSize; ++i)
        {
            weights[i].playerIndex = i;
            weights[i].performanceScore = performanceScores[i];
        }
        
        // Compute team statistics
        double mean = ComputeMean(performanceScores);
        double stddev = ComputeStdDev(performanceScores, mean);
        
        // Compute z-scores and weights for each player
        for (int i = 0; i < teamSize; ++i)
        {
            // z_i = (p_i - mean) / stddev
            weights[i].zScore = ComputeZScore(performanceScores[i], mean, stddev);
            
            // w_i^raw = 1 + alpha * z_i
            weights[i].rawWeight = ComputeRawWeight(weights[i].zScore, alpha);
            
            // Clamp to [wMin, wMax]
            weights[i].clampedWeight = ClampWeight(weights[i].rawWeight, wMin, wMax);
        }
        
        // Normalize so average weight = 1.0
        NormalizeWeights(weights);
        
        return weights;
    }
    
    double PerformanceWeighting::ComputeMean(const std::vector<double>& scores)
    {
        if (scores.empty())
        {
            return 0.0;
        }
        
        double sum = 0.0;
        for (double score : scores)
        {
            sum += score;
        }
        
        return sum / scores.size();
    }
    
    double PerformanceWeighting::ComputeStdDev(const std::vector<double>& scores, double mean)
    {
        if (scores.empty())
        {
            return TeamGlicko2::kEpsilon;
        }
        
        // s_T = sqrt((1/|T|) * sum((p_i - mean)^2)) + epsilon
        double sumSquares = 0.0;
        for (double score : scores)
        {
            double diff = score - mean;
            sumSquares += diff * diff;
        }
        
        double variance = sumSquares / scores.size();
        return std::sqrt(variance) + TeamGlicko2::kEpsilon;
    }
    
    double PerformanceWeighting::ComputeZScore(double score, double mean, double stddev)
    {
        // z_i = (p_i - mean) / stddev
        return (score - mean) / stddev;
    }
    
    double PerformanceWeighting::ComputeRawWeight(double zScore, double alpha)
    {
        // w_i^raw = 1 + alpha * z_i
        return 1.0 + alpha * zScore;
    }
    
    double PerformanceWeighting::ClampWeight(double weight, double wMin, double wMax)
    {
        // w_i = min(wMax, max(wMin, w_i^raw))
        return std::min(wMax, std::max(wMin, weight));
    }
    
    void PerformanceWeighting::NormalizeWeights(std::vector<PlayerWeight>& weights)
    {
        if (weights.empty())
        {
            return;
        }
        
        // Compute sum of clamped weights
        double sumWeights = 0.0;
        for (const auto& w : weights)
        {
            sumWeights += w.clampedWeight;
        }
        
        // Normalize: w_i_tilde = w_i * |T| / sum(w_j)
        int teamSize = weights.size();
        double normalizationFactor = teamSize / sumWeights;
        
        for (auto& w : weights)
        {
            w.normalizedWeight = w.clampedWeight * normalizationFactor;
        }
    }
    
} // namespace TeamGlicko2
