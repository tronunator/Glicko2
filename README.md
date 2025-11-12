# Team-Based Glicko-2 Rating System with Performance Weighting

A C++ implementation of a competitive rating system for team-based multiplayer games (4v4, 5v5, NvsN), based on the Glicko-2 algorithm with individual performance weighting.

## Overview

This system extends the standard Glicko-2 rating algorithm to support:
- **Team-based matchmaking** (two teams per match)
- **Per-player performance weighting** (stronger contribution for players who carry, weaker for those who underperform)
- **Fair rating updates** where win/loss is the primary driver, but individual performance modulates the magnitude of rating changes

### Key Features

- ✅ Full Glicko-2 implementation with volatility tracking
- ✅ Team aggregation for NvsN matches
- ✅ Performance-weighted rating updates
- ✅ Normalized weighting (team average always = 1.0)
- ✅ Configurable sensitivity parameters
- ✅ Optional rating change clamping
- ✅ Support for wins, losses, and draws

## Project Structure

```
ElimRank/
├── include/                    # Header files
│   ├── TeamGlicko2Config.h         # Configuration constants and parameters
│   ├── TeamGlickoRating.h          # Individual player rating state
│   ├── TeamRatingAggregator.h      # Team-level rating aggregation
│   ├── PerformanceWeighting.h      # Performance weight calculation
│   └── TeamGlicko2System.h         # Main match processing system
├── src/                        # Implementation files
│   ├── TeamGlickoRating.cpp
│   ├── TeamRatingAggregator.cpp
│   ├── PerformanceWeighting.cpp
│   └── TeamGlicko2System.cpp
├── examples/                   # Example programs
│   └── example_usage.cpp           # Comprehensive examples
├── docs/                       # Documentation
│   └── implementation.latex        # Mathematical specification
├── build/                      # Build artifacts (generated, gitignored)
├── Makefile                    # Build system
├── .gitignore                  # Git ignore rules
└── README.md                   # This file
```

## Building

### Using Make (Windows/Linux)

```bash
# Build the example
make

# Run the example
make run

# Clean build artifacts
make clean

# Deep clean (remove build directory)
make distclean

# Show available targets
make help
```

### Manual Compilation (Windows)

```bash
# Using MSVC
cl /EHsc /std:c++14 /Iinclude src/TeamGlickoRating.cpp src/TeamRatingAggregator.cpp src/PerformanceWeighting.cpp src/TeamGlicko2System.cpp examples/example_usage.cpp /Fe:build/TeamGlicko2Example.exe

# Using MinGW
g++ -std=c++14 -O2 -Iinclude src/TeamGlickoRating.cpp src/TeamRatingAggregator.cpp src/PerformanceWeighting.cpp src/TeamGlicko2System.cpp examples/example_usage.cpp -o build/TeamGlicko2Example.exe
```

### Manual Compilation (Linux/Mac)

```bash
g++ -std=c++14 -O2 -Iinclude src/TeamGlickoRating.cpp src/TeamRatingAggregator.cpp src/PerformanceWeighting.cpp src/TeamGlicko2System.cpp examples/example_usage.cpp -o build/TeamGlicko2Example -lm
```

## Usage

### Basic Example

```cpp
#include "TeamGlicko2System.h"

using namespace TeamGlicko2;

// Create a match result
MatchResult match;

// Add Team A players with their ratings and performance scores
match.teamA.push_back(MatchPlayer(
    PlayerRating(1500, 200),  // Rating, RD
    25.5                       // Performance score
));
// ... add more players

// Add Team B players
match.teamB.push_back(MatchPlayer(
    PlayerRating(1500, 200),
    22.3
));
// ... add more players

// Set match outcome (1.0 = win, 0.0 = loss, 0.5 = draw)
match.scoreA = 1.0;  // Team A wins
match.scoreB = 0.0;  // Team B loses

// Process the match (updates all player ratings in place)
TeamGlicko2System::ProcessMatch(match);

// Access updated ratings
std::cout << "Player 1 new rating: " << match.teamA[0].rating.GetRating() << std::endl;
```

### Computing Performance Scores

Performance scores should reflect individual contribution to the match. Example:

```cpp
double ComputePerformanceScore(int kills, int deaths, double damage, double objectiveScore)
{
    return kKillWeight * kills
         + kDeathWeight * deaths      // typically negative
         + kDamageWeight * damage     // typically scaled down
         + kObjectiveWeight * objectiveScore;
}
```

**Important:** Only relative performance within each team matters. The system normalizes weights so the average is always 1.0.

## Configuration

All tunable parameters are in `TeamGlicko2Config.h`:

### Glicko-2 Base Parameters

- `kDefaultRating` (1500.0): Initial rating for new players
- `kDefaultRD` (350.0): Initial rating deviation (uncertainty)
- `kDefaultVolatility` (0.06): Initial volatility
- `kTau` (0.5): System constant controlling volatility change rate
- `kScale` (173.7178): Glicko-1 to Glicko-2 conversion factor

### Performance Weighting Parameters

- `kAlpha` (0.2): Controls how strongly performance influences rating
  - Range: [0.1, 0.3] recommended
  - Higher = more impact from individual performance
  - Lower = more team-based rating changes

- `kWeightMin` (0.5): Minimum performance weight
  - Prevents extreme negative weighting for underperformers

- `kWeightMax` (1.5): Maximum performance weight
  - Caps benefit from extreme carry performance

- `kEpsilon` (1e-6): Small constant to avoid division by zero

### Rating Change Clamping

- `kEnableRatingClamp` (true): Enable/disable clamping
- `kMaxRatingChange` (1.73): Maximum rating change per match (Glicko-2 scale)
  - Approximately 300 points in Glicko-1 scale

## Algorithm Details

The implementation follows the specification in `docs/implementation.latex`. Key steps:

1. **Convert to Glicko-2 Scale**: R, RD → μ, φ
2. **Compute Team Statistics**: Aggregate opponent team's μ and φ
3. **Calculate Performance Weights**: 
   - Compute z-scores within each team
   - Apply α scaling factor
   - Clamp to [w_min, w_max]
   - Normalize so average = 1.0
4. **Standard Glicko-2 Update**: Each player vs aggregated opponent
5. **Apply Performance Weighting**: Scale rating change by normalized weight
6. **Optional Clamping**: Limit extreme rating swings
7. **Convert Back**: μ, φ → R, RD

### Performance Weighting Formula

For each player i in team T:

```
z_i = (p_i - mean(T)) / stddev(T)
w_i^raw = 1 + α * z_i
w_i = clamp(w_i^raw, w_min, w_max)
w_i_tilde = w_i * |T| / sum(w_j)
```

Final rating change:
```
μ' = μ + w_i_tilde * (μ* - μ)
```

Where μ* is the standard Glicko-2 update result.

## Examples

### Example Program

Run `example_usage` to see four comprehensive examples:

1. **Balanced 4v4**: Equal teams, Team A wins
2. **Upset Victory**: Lower-rated team defeats favorites
3. **Extreme Difference 5v5**: One player hard carries their team
4. **Draw Match**: Tied game with slight rating adjustments

Each example demonstrates:
- Rating changes based on win/loss
- Performance weighting within teams
- Top performers gain more (or lose less)
- Bottom performers gain less (or lose more)

### Batch Processor

Process historical match data from CSV:

```bash
batch_processor [input.csv] [output.csv]
```

The input CSV should contain columns:
- `MATCHID`: Match identifier
- `PLAYERID`: Unique player identifier
- `PlayerName`: Human-readable player name
- `TEAM`: "Red" or "Blue"
- `KILLS`, `DEATH`, `DAMAGE`, `Score`: Performance statistics
- `Winner`: "Red", "Blue", or "Draw"

The output CSV will contain rating evolution for each player across all matches, including:
- Rating before/after each match
- Rating deviation (RD)
- Rating change
- Performance score

## Design Notes

### Win/Loss vs Performance

- **Win/loss is the primary driver** of rating changes (Glicko-2 core)
- **Performance only modulates magnitude** of the change
- Team average weight is always 1.0 (normalized)
- System maintains fairness while rewarding individual skill

### Role Balance

Performance scores should be designed to fairly evaluate different roles:
- Support players: Weight objectives and assists heavily
- Damage dealers: Balance kills with deaths (K/D)
- Objective players: Emphasize objective contribution

### Uncertainty (RD)

- Rating Deviation decreases with each match (increased certainty)
- New players have high RD → larger rating swings
- Experienced players have low RD → smaller rating swings
- Use `Decay()` if player doesn't play for extended period

## Theory and References

This implementation is based on:

- **Glicko-2 Rating System** by Mark Glickman
  - [Glicko-2 Paper](http://www.glicko.net/glicko/glicko2.pdf)
  - [Official Website](http://www.glicko.net/glicko.html)

- **Team-Based Extension** with performance weighting
  - See `docs/implementation.latex` for mathematical specification
  - Original contribution for NvsN team games

