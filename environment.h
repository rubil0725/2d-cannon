#pragma once
#include "raylib.h"
// Forward declarations to avoid heavy includes in the header
struct Player;
class GamePlaySystem;
#include <vector>

class Environment
{
public:
	void Init(int screenW, int screenH);
	void Update(Player &gameP1, Player &gameP2, class GamePlaySystem &gameplay, float deltaTime);
	void Draw();

	float wind = 0.0f; // current wind force applied to projectiles (pixels/sec^2 in X)

	// apply circular explosion to terrain (carves out a crater)
	void ApplyExplosion(float x, float y, float radius);

	// helper to query terrain height at integer x
	float GetTerrainHeight(int x) const;

private:
	// background / sun
	bool sunisintheSky = true;
	float cycleStartTime = 0.0f;
	int SCREEN_W = 800;
	int SCREEN_H = 450;
	Color bgColor1{};
	Color bgColor2{};

	// terrain
	std::vector<float> terrain;
	std::vector<float> bgTerrain;

	// (DemoPlayer and Projectile were unused and removed to reduce coupling)
	struct Explosion { float x=0,y=0,radius=0,timer=0; bool active=false; float maxRadius=30; } explosion;

	float GROUND_Y = 350.0f;
	float sunCenterX = 0, sunHorizonY=0, sunHorizontalRadius=0, sunVerticalRadius=0, sunDiscRadius=0;
	// helpers
	float catmullRom(float p0, float p1, float p2, float p3, float t);
	void generateTerrain();
	void generateBgTerrain();
};
