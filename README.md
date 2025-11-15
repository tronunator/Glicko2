# Team-Based Glicko-2 Rating System

A C++ implementation of a competitive rating system for team-based multiplayer games (4v4, 5v5, NvsN), extending Glicko-2 with sign-aware performance scaling.

## Configuration

Key parameters in `TeamGlicko2Config.h`:

- `kDefaultRating` (1400): Initial rating
- `kDefaultRD` (350): Initial rating deviation
- `kBeta` (0.2): Performance scaling sensitivity [0.15-0.30]
- `kScaleMin/Max` (0.5/1.5): Performance scaling bounds
- `kPerfTargetWindow` (10): EMA window for recent performance
- `kLambda` (0.8): Team balancing uncertainty weight

## Algorithm

Full mathematical specification in **`docs/implementation.pdf`**.

Key concepts:
- **Sign-aware scaling**: `f = 1 + β·sign(Δμ)·z` ensures proper incentives in wins and losses
- **Z-scores**: Performance measured relative to teammates
- **Recent performance**: EMA tracks short-term form over ~10 games
- **Effective rating**: `R_eff = R + w·(R_recent - R)` where `w` depends on RD
- **Team balancing**: Uses effective ratings (no additional risk adjustment needed)

## Examples

See `examples` for usage demos

## References
Full mathematical specification in `docs/implementation.pdf`.
Based on **Glicko-2** by Mark Glickman ([paper](http://www.glicko.net/glicko/glicko2.pdf)), extended for team games with sign-aware performance scaling. 
