#ifndef TEAM_GLICKO_RATING_H
#define TEAM_GLICKO_RATING_H

#include <iostream>
#include <cmath>
#include "TeamGlicko2Config.h"

namespace TeamGlicko2
{
    /// Represents a player's Glicko-2 rating state
    /// Maintains both Glicko-1 scale (R, RD) and Glicko-2 scale (mu, phi)
    class PlayerRating
    {
    public:
        /// Default constructor with initial rating values
        PlayerRating(double rating = TeamGlicko2::kDefaultRating,
                    double ratingDeviation = TeamGlicko2::kDefaultRD,
                    double volatility = TeamGlicko2::kDefaultVolatility);
        
        /// Copy constructor
        PlayerRating(const PlayerRating& other);
        
        /// Assignment operator
        PlayerRating& operator=(const PlayerRating& other);
        
        // ========== Glicko-1 Scale Getters (R, RD) ==========
        
        /// Get rating in Glicko-1 scale
        double GetRating() const { return mu * TeamGlicko2::kScale + TeamGlicko2::kDefaultRating; }
        
        /// Get rating deviation in Glicko-1 scale
        double GetRD() const { return phi * TeamGlicko2::kScale; }
        
        /// Get volatility (same in both scales)
        double GetVolatility() const { return sigma; }
        
        // ========== Glicko-2 Scale Getters (mu, phi) ==========
        
        /// Get rating in Glicko-2 scale (mu)
        double GetMu() const { return mu; }
        
        /// Get rating deviation in Glicko-2 scale (phi)
        double GetPhi() const { return phi; }
        
        /// Get volatility in Glicko-2 scale (sigma)
        double GetSigma() const { return sigma; }
        
        // ========== Setters ==========
        
        /// Set rating from Glicko-1 scale
        void SetRating(double rating);
        
        /// Set rating deviation from Glicko-1 scale
        void SetRD(double rd);
        
        /// Set volatility
        void SetVolatility(double vol) { sigma = vol; }
        
        /// Set rating from Glicko-2 scale (mu)
        void SetMu(double muValue) { mu = muValue; }
        
        /// Set rating deviation from Glicko-2 scale (phi)
        void SetPhi(double phiValue) { phi = phiValue; }
        
        /// Set volatility from Glicko-2 scale (sigma)
        void SetSigma(double sigmaValue) { sigma = sigmaValue; }
        
        // ========== Utility Functions ==========
        
        /// Computes g(phi) function used in Glicko-2 calculations
        /// g(phi) = 1 / sqrt(1 + 3*phi^2 / pi^2)
        double ComputeG() const;
        
        /// Computes expected score E against another player
        /// E = 1 / (1 + exp(-g(phi_opp) * (mu - mu_opp)))
        double ComputeExpectedScore(double muOpp, double gOpp) const;
        
        /// Print rating information
        friend std::ostream& operator<<(std::ostream& os, const PlayerRating& rating);
        
    private:
        /// Internal rating (Glicko-2 scale)
        double mu;
        
        /// Internal rating deviation (Glicko-2 scale)
        double phi;
        
        /// Rating volatility
        double sigma;
    };
    
} // namespace TeamGlicko2

#endif // TEAM_GLICKO_RATING_H
