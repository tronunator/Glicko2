#include "TeamGlickoRating.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace TeamGlicko2
{
    PlayerRating::PlayerRating(double rating, double ratingDeviation, double volatility)
    {
        // Convert from Glicko-1 scale to Glicko-2 scale
        mu = (rating - TeamGlicko2::kDefaultRating) / TeamGlicko2::kScale;
        phi = ratingDeviation / TeamGlicko2::kScale;
        sigma = volatility;
    }
    
    PlayerRating::PlayerRating(const PlayerRating& other)
        : mu(other.mu), phi(other.phi), sigma(other.sigma)
    {
    }
    
    PlayerRating& PlayerRating::operator=(const PlayerRating& other)
    {
        if (this != &other)
        {
            mu = other.mu;
            phi = other.phi;
            sigma = other.sigma;
        }
        return *this;
    }
    
    void PlayerRating::SetRating(double rating)
    {
        mu = (rating - TeamGlicko2::kDefaultRating) / TeamGlicko2::kScale;
    }
    
    void PlayerRating::SetRD(double rd)
    {
        phi = rd / TeamGlicko2::kScale;
    }
    
    double PlayerRating::ComputeG() const
    {
        // g(phi) = 1 / sqrt(1 + 3*phi^2 / pi^2)
        double phiSquared = phi * phi;
        return 1.0 / std::sqrt(1.0 + 3.0 * phiSquared / (M_PI * M_PI));
    }
    
    double PlayerRating::ComputeExpectedScore(double muOpp, double gOpp) const
    {
        // E = 1 / (1 + exp(-g(phi_opp) * (mu - mu_opp)))
        double exponent = -gOpp * (mu - muOpp);
        return 1.0 / (1.0 + std::exp(exponent));
    }
    
    std::ostream& operator<<(std::ostream& os, const PlayerRating& rating)
    {
        os << "Rating: " << rating.GetRating() 
           << ", RD: " << rating.GetRD() 
           << ", Volatility: " << rating.GetVolatility();
        return os;
    }
    
} // namespace TeamGlicko2
