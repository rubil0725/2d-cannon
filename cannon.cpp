#include "cannon.h"
#include "GameplaySystem.h"
#include <cmath>
#include <algorithm>

Vector2 Cannon::GetMuzzlePos() const {
    float rad = angleDeg * DEG2RAD;
    Vector2 turretCenter = { groundPos.x, groundPos.y - BASE_HEIGHT - TURRET_RADIUS * 0.4f };
    return {
        turretCenter.x + cosf(rad) * BARREL_LENGTH,
        turretCenter.y - sinf(rad) * BARREL_LENGTH
    };
}

Rectangle Cannon::GetHitbox() const {
    float w = BASE_WIDTH;
    float h = BASE_HEIGHT + TURRET_RADIUS * 1.6f;
    return { groundPos.x - w / 2.0f, groundPos.y - h, w, h };
}

void Cannon::Draw() const {

    Vector2 turretCenter = { groundPos.x, groundPos.y - BASE_HEIGHT - TURRET_RADIUS * 0.4f };

    // Soft contact shadow grounds the cannon visually against the terrain
    DrawEllipse((int)groundPos.x, (int)groundPos.y + 2, BASE_WIDTH * 0.62f, 5.0f, Fade(BLACK, 0.35f));

    // Tracked base: a two-tone rounded rectangle (darker underside, lighter
    // top) for a bit of shading, plus a tread strip and road wheels.
    Rectangle baseRect = { groundPos.x - BASE_WIDTH / 2.0f, groundPos.y - BASE_HEIGHT,
                            BASE_WIDTH, BASE_HEIGHT };
    Color bodyShade = { (unsigned char)(bodyColor.r * 0.65f),
                        (unsigned char)(bodyColor.g * 0.65f),
                        (unsigned char)(bodyColor.b * 0.65f), 255 };
    DrawRectangleRounded(baseRect, 0.4f, 8, bodyShade);
    Rectangle topStrip = { baseRect.x, baseRect.y, baseRect.width, baseRect.height * 0.55f };
    DrawRectangleRounded(topStrip, 0.5f, 8, bodyColor);

    // Tread strip along the bottom edge
    DrawRectangle((int)baseRect.x - 2, (int)groundPos.y - 6, (int)BASE_WIDTH + 4, 6, { 25, 25, 28, 255 });

    // Road wheels (3 per side) with a lighter hub for a bit of depth
    for (int w = 0; w < 3; w++) {
        float wx = baseRect.x + 6.0f + w * (BASE_WIDTH - 12.0f) / 2.0f;
        DrawCircle((int)wx, (int)groundPos.y - 3, 7, BLACK);
        DrawCircle((int)wx, (int)groundPos.y - 3, 4, DARKGRAY);
        DrawCircle((int)wx - 1, (int)groundPos.y - 4, 1.5f, LIGHTGRAY);
    }

    // Barrel: drawn tapered (thicker near the turret, narrower at the tip)
    // using a few stacked segments instead of one uniform-thickness line.
    float rad = angleDeg * DEG2RAD;
    Vector2 dir = { cosf(rad), -sinf(rad) };
    Color barrelShade = { (unsigned char)(barrelColor.r * 0.7f),
                          (unsigned char)(barrelColor.g * 0.7f),
                          (unsigned char)(barrelColor.b * 0.7f), 255 };
    const int barrelSegs = 4;
    for (int s = 0; s < barrelSegs; s++) {
        float t0 = (float)s / barrelSegs;
        float t1 = (float)(s + 1) / barrelSegs;
        Vector2 p0 = { turretCenter.x + dir.x * BARREL_LENGTH * t0, turretCenter.y + dir.y * BARREL_LENGTH * t0 };
        Vector2 p1 = { turretCenter.x + dir.x * BARREL_LENGTH * t1, turretCenter.y + dir.y * BARREL_LENGTH * t1 };
        float thick = BARREL_THICK * (1.0f - 0.28f * t0);
        DrawLineEx(p0, p1, thick, s % 2 == 0 ? barrelColor : barrelShade);
    }
    Vector2 barrelEnd = { turretCenter.x + dir.x * BARREL_LENGTH, turretCenter.y + dir.y * BARREL_LENGTH };
    // Muzzle brake: a slightly wider dark ring capping the tip
    DrawCircle((int)barrelEnd.x, (int)barrelEnd.y, BARREL_THICK * 0.62f, { 30, 30, 32, 255 });
    DrawCircle((int)barrelEnd.x, (int)barrelEnd.y, BARREL_THICK * 0.4f, barrelShade);

    // Turret dome on top of the base, with a highlight for a rounded, lit look
    DrawCircle((int)turretCenter.x, (int)turretCenter.y, TURRET_RADIUS, bodyColor);
    DrawCircle((int)turretCenter.x, (int)turretCenter.y, TURRET_RADIUS, Fade(BLACK, 0.12f));
    DrawCircle((int)(turretCenter.x - TURRET_RADIUS * 0.3f), (int)(turretCenter.y - TURRET_RADIUS * 0.35f),
               TURRET_RADIUS * 0.35f, Fade(WHITE, 0.28f)); // shine highlight
    DrawCircleLines((int)turretCenter.x, (int)turretCenter.y, TURRET_RADIUS, Fade(BLACK, 0.4f));

    // Small antenna for a bit of silhouette detail
    Vector2 antennaBase = { turretCenter.x - TURRET_RADIUS * 0.5f, turretCenter.y - TURRET_RADIUS * 0.3f };
    Vector2 antennaTip  = { antennaBase.x - 4.0f, antennaBase.y - 12.0f };
    DrawLineEx(antennaBase, antennaTip, 1.5f, DARKGRAY);
    DrawCircle((int)antennaTip.x, (int)antennaTip.y, 1.8f, RED);

    // Player number badge with a small background chip so it stays readable
    // against any body color.
    const char* label = (playerIndex == 0) ? "P1" : "P2";
    int textW = MeasureText(label, 14);
    Rectangle chip = { groundPos.x - textW / 2.0f - 4, groundPos.y + 2, (float)textW + 8, 16 };
    DrawRectangleRounded(chip, 0.5f, 6, Fade(WHITE, 0.85f));
    DrawText(label, (int)(groundPos.x - textW / 2.0f), (int)(groundPos.y + 4), 14, BLACK);
}

void Cannon::DrawHealthBar(int healthValue, int maxHealthValue) const {
    float barW = BASE_WIDTH + 10.0f;
    float barH = 6.0f;
    float x = groundPos.x - barW / 2.0f;
    float y = groundPos.y - BASE_HEIGHT - TURRET_RADIUS * 2.0f - 14.0f;

    float pct = (maxHealthValue > 0) ? (float)healthValue / (float)maxHealthValue : 0.0f;
    pct = std::max(0.0f, std::min(1.0f, pct));

    Color fillColor = (pct > 0.5f) ? GREEN : (pct > 0.25f) ? ORANGE : RED;

    DrawRectangle((int)x, (int)y, (int)barW, (int)barH, Fade(BLACK, 0.5f));
    DrawRectangle((int)x + 1, (int)y + 1, (int)((barW - 2) * pct), (int)(barH - 2), fillColor);
    DrawRectangleLines((int)x, (int)y, (int)barW, (int)barH, BLACK);
}

void CannonManager::SpawnCannons(Terrain& terrain, int seed, int minGap, int maxGap) {
    srand(seed + 777); // distinct offset so cannon placement != terrain/obstacle rolls

    int margin = TERRAIN_WIDTH / 12; // keep cannons off the very edge of the map

    // Randomize the distance between the two cannons every match.
    int gap = minGap + rand() % (maxGap - minGap + 1);
    gap = std::min(gap, TERRAIN_WIDTH - margin * 2); // never wider than the playable field

    // Randomize WHERE that pair sits (not always dead-center), so the
    // battlefield layout varies too, while both cannons stay in-bounds.
    int minMid = margin + gap / 2;
    int maxMid = TERRAIN_WIDTH - margin - gap / 2;
    int mid = (maxMid > minMid) ? (minMid + rand() % (maxMid - minMid + 1)) : TERRAIN_WIDTH / 2;

    int leftX  = mid - gap / 2;
    int rightX = mid + gap / 2;
    leftX  = std::max(margin, std::min(leftX,  TERRAIN_WIDTH - margin));
    rightX = std::max(margin, std::min(rightX, TERRAIN_WIDTH - margin));

    cannons[0].groundPos    = { (float)leftX,  (float)terrain.heights[leftX] };
    cannons[0].angleDeg     = 45.0f;   // aiming up-right toward the opponent
    cannons[0].power        = 55.0f;
    // cannons internal health removed; no-op initialization
    cannons[0].bodyColor    = { 60, 110, 200, 255 };  // blue = player 1
    cannons[0].barrelColor  = { 40, 70,  140, 255 };
    cannons[0].playerIndex  = 0;

    cannons[1].groundPos    = { (float)rightX, (float)terrain.heights[rightX] };
    cannons[1].angleDeg     = 135.0f;  // aiming up-left toward the opponent
    cannons[1].power        = 55.0f;
    // cannons internal health removed; no-op initialization
    cannons[1].bodyColor    = { 200, 70, 60, 255 };   // red = player 2
    cannons[1].barrelColor  = { 140, 45, 40, 255 };
    cannons[1].playerIndex  = 1;
}

void CannonManager::SettleOnTerrain(Terrain& terrain) {
    for (auto& c : cannons) {
        int x = (int)c.groundPos.x;
        x = std::max(0, std::min(x, TERRAIN_WIDTH - 1));
        c.groundPos.y = (float)terrain.heights[x];
    }
}

void CannonManager::Draw() const {
    for (int i = 0; i < 2; ++i) {
        const Cannon& c = cannons[i];
        // If Player pointer is present and player is dead, skip drawing the cannon
        const Player* p = players[i];
        if (p && p->health <= 0) continue;
        c.Draw();
        if (p) {
            c.DrawHealthBar(p->health, p->maxHealth);
        }
        else {
            c.DrawHealthBar(100, 100);
        }
    }
}
