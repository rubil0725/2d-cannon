#pragma once
#include "raylib.h"
#include <vector>

// How wide the terrain is (one height value per pixel column)
const int TERRAIN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Represents one "chunk" of terrain as a rectangle
// Used for collision detection
struct TerrainSegment {
    Rectangle rect;   // x, y, width, height in world space
    bool active;      // false = this segment was destroyed
};

class Terrain {
public:
    // The heightmap: heights[x] = the Y position of the ground at column x
    // Lower Y = higher on screen (raylib Y axis goes DOWN)
    std::vector<int> heights;

    // Flat collision rectangles derived from the heightmap
    std::vector<TerrainSegment> segments;

    Terrain();

    // Generate a fresh terrain using sine waves + random bumps
    void Generate(int seed);

    // Break terrain at a position (simulate cannonball impact)
    void Destroy(int centerX, int radius);

    // Rebuild the segment list from current heights
    void RebuildSegments();

    // Draw the terrain as a filled polygon
    void Draw();
};
