#ifndef TEAM_GLICKO2_CONFIG_H
#define TEAM_GLICKO2_CONFIG_H

namespace TeamGlicko2
{
    // ========== Glicko-2 Base Parameters ==========
    
    /// Default initial rating (R_0)
    static const double kDefaultRating = 1500.0;
    
    /// Default initial rating deviation (RD_0)
    static const double kDefaultRD = 350.0;
    
    /// Default initial volatility (sigma_0)
    static const double kDefaultVolatility = 0.06;
    
    /// Gamma: Risk-adjustment parameter for player score
    /// S_i = R_i - gamma * RD_i
    /// Range: [0.0, ∞)
    /// - 0.0 = ignore uncertainty (use raw rating only)
    /// - 0.5 = moderate risk adjustment (recommended)
    /// - 1.0 = strong penalty for uncertain ratings
    static const double kGamma = 0.5;
    
    /// Lambda: Team uncertainty balance weight
    /// Objective = |avg(S_A) - avg(S_B)| + lambda * |avg(U_A) - avg(U_B)|
    /// Uses averages for fair comparison in uneven teams
    /// Range: [0.0, ∞)
    /// - 0.0 = ignore team uncertainty balance
    /// - 0.1 = small preference for balanced uncertainty (recommended)
    /// - 1.0 = strong preference for balanced uncertainty
    static const double kLambda = 0.1;
    
    /// Glicko-1 to Glicko-2 scale factor (173.7178)
    static const double kScale = 173.7178;
    
    /// System constant (tau) - controls volatility change rate
    static const double kTau = 0.5;
    
    /// Convergence tolerance for volatility update (epsilon)
    static const double kConvergence = 0.000001;
    
    
    // ========== Performance Weighting Parameters ==========
    
    /// Alpha: controls how strongly performance influences rating changes
    /// Recommended range: [0.1, 0.3]
    static const double kAlpha = 0.2;
    
    /// Minimum performance weight
    static const double kWeightMin = 0.5;
    
    /// Maximum performance weight
    static const double kWeightMax = 1.5;
    
    /// Small constant to avoid division by zero in standard deviation
    static const double kEpsilon = 1e-6;
    
    
    // ========== Optional Rating Change Clamping ==========
    
    /// Enable clamping of rating changes (set to false to disable)
    static const bool kEnableRatingClamp = true;
    
    /// Maximum rating change per match (in Glicko-2 scale)
    /// This corresponds to approximately 300 points in Glicko-1 scale
    static const double kMaxRatingChange = 1.73;
    
    
    // ========== Match Outcome Scores ==========
    
    /// Score for winning team
    static const double kWinScore = 1.0;
    
    /// Score for losing team
    static const double kLossScore = 0.0;
    
    /// Score for draw (if supported)
    static const double kDrawScore = 0.5;


    // ========== Performance Score Calculation ==========
    // These weights compute: (Kills*250) + (Deaths*kDeathWeight) + (Damage*0.5) + (Score*0.1)
    // Adjust based on your game's balance
    
    /// Weight for kills in performance score
    static const double kKillWeight = 1;
    
    /// Weight for deaths in performance score (negative penalty)
    /// -100 means 1 death ≈ 2.5 kills penalty
    /// -50 would make deaths less punishing
    static const double kDeathWeight = -1;
    
    /// Weight for damage in performance score
    /// 0.5 means 500 damage ≈ 1 kill
    static const double kDamageWeight = 0.01;
    
    /// Weight for objective score in performance score
    static const double kObjectiveWeight = 0;
}

#endif // TEAM_GLICKO2_CONFIG_H
