#pragma once
#include "raylib.h"
#include "terrain.h"

// ── Cannon ─────────────────────────────────────────────────────────────────
// One player's cannon: a base (tracked body) + turret + barrel, drawn as
// actual shapes rather than a placeholder rectangle, plus health tracking
// so it can be damaged and eventually destroyed.
struct Cannon {
    Vector2 groundPos;   // where the cannon sits (bottom-center, on the terrain)
    float   angleDeg;    // barrel angle in degrees. 0 = pointing right, 180 = pointing left
    float   power;       // 0-100 shot power, used by whatever fires the projectile
    int     health;
    int     maxHealth;
    Color   bodyColor;
    Color   barrelColor;
    int     playerIndex; // 0 = left player, 1 = right player
    bool    active;      // false once destroyed

    static constexpr float BASE_WIDTH  = 46.0f;
    static constexpr float BASE_HEIGHT = 18.0f;
    static constexpr float TURRET_RADIUS = 14.0f;
    static constexpr float BARREL_LENGTH = 34.0f;
    static constexpr float BARREL_THICK  = 9.0f;

    // World-space position the barrel tip fires from (where projectiles should spawn).
    Vector2 GetMuzzlePos() const;

    // Rectangle used for incoming-projectile collision checks (roughly the base+turret).
    Rectangle GetHitbox() const;

    void TakeDamage(int amount); // sets active=false if health drops to 0 or below

    void Draw() const;         // draws base, turret and barrel
    void DrawHealthBar() const; // small bar floating above the cannon
};

// ── CannonManager ──────────────────────────────────────────────────────────
// Owns both players' cannons, places them on the terrain at the start of a
// match with a RANDOMIZED distance between them (so every match feels
// different), and keeps them glued to the ground if terrain deforms nearby.
class CannonManager {
public:
    Cannon cannons[2];

    // Places both cannons.
    //   minGap / maxGap: the randomized horizontal distance (in pixels)
    //   kept between the two cannons is chosen uniformly from this range,
    //   and their shared midpoint is also jittered, so no two matches
    //   look the same.
    void SpawnCannons(Terrain& terrain, int seed,
                       int minGap = 300, int maxGap = 900);

    // Re-snaps both cannons to the terrain height under them.
    // Call after Terrain::Destroy() runs near a cannon so it doesn't float.
    void SettleOnTerrain(Terrain& terrain);

    void Draw() const;
};
