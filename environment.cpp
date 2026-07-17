#include "environment.h"
#include "raylib.h"
#include <cmath>
#include <cstdlib>

void Environment::Init(int screenW, int screenH)
{
	SCREEN_W = screenW; SCREEN_H = screenH;
	terrain.assign(SCREEN_W, 0.0f);
	bgTerrain.assign(SCREEN_W, 0.0f);
	GROUND_Y = SCREEN_H * 0.77f;
	// sun setup
	sunCenterX = SCREEN_W / 2.0f;
	sunHorizonY = SCREEN_H * 0.47f;
	sunHorizontalRadius = SCREEN_W / 2.0f;
	sunVerticalRadius = SCREEN_W / 5.0f;
	sunDiscRadius = SCREEN_W * 0.025f;

	generateTerrain();
	generateBgTerrain();

	cycleStartTime = GetTime();
	wind = 0.0f;
}

void Environment::ApplyExplosion(float x, float y, float radius)
{
	// carve terrain: lower terrain heights within radius by a simple circular profile
	int minX = (int)std::max(0.0f, floorf(x - radius));
	int maxX = (int)std::min((float)(SCREEN_W - 1), ceilf(x + radius));
	for (int ix = minX; ix <= maxX; ++ix)
	{
		float dx = ix - x;
		float dist = fabsf(dx);
		if (dist > radius) continue;
		// simple crater depth proportional to (cosine falloff)
		float fall = cosf((dist / radius) * (3.14159265f / 2.0f));
		float depth = fall * radius * 0.6f; // scale crater depth
		terrain[ix] += depth; // increase y (downwards) to carve
		if (terrain[ix] > SCREEN_H) terrain[ix] = SCREEN_H;
	}
}


float Environment::GetTerrainHeight(int x) const
{
	if (x < 0) return (float)SCREEN_H;
	if (x >= SCREEN_W) return (float)SCREEN_H;
	return terrain[x];
}

float Environment::catmullRom(float p0, float p1, float p2, float p3, float t)
{
	return 0.5f * (
		(-t * t * t + 2 * t * t - t) * p0 +
		(3 * t * t * t - 5 * t * t + 2) * p1 +
		(-3 * t * t * t + 4 * t * t + t) * p2 +
		(t * t * t - t * t) * p3
		);
}

void Environment::generateTerrain()
{
	const int NUM_CP = 8;
	std::vector<float> cpX(NUM_CP), cpY(NUM_CP);
	int TERRAIN_MIN = SCREEN_H * 0.33f;
	int TERRAIN_MAX = SCREEN_H * 0.78f;
	for (int i = 0; i < NUM_CP; i++) cpX[i] = (SCREEN_W / (float)(NUM_CP - 1)) * i;
	for (int i = 0; i < NUM_CP; i++) cpY[i] = GetRandomValue(TERRAIN_MIN, TERRAIN_MAX);

	for (int x = 0; x < SCREEN_W; x++)
	{
		int seg = 0;
		for (int i = 0; i < NUM_CP - 1; i++)
		{
			if (x >= cpX[i] && x < cpX[i + 1]) { seg = i; break; }
		}
		float t = (x - cpX[seg]) / (cpX[seg + 1] - cpX[seg]);
		int i0 = (seg - 1 < 0) ? 0 : seg - 1;
		int i1 = seg;
		int i2 = seg + 1;
		int i3 = (seg + 2 >= NUM_CP) ? NUM_CP - 1 : seg + 2;
		terrain[x] = catmullRom(cpY[i0], cpY[i1], cpY[i2], cpY[i3], t);
	}
}

void Environment::generateBgTerrain()
{
	int BG_MIN = SCREEN_H * 0.22f;
	int BG_MAX = SCREEN_H * 0.47f;
	bgTerrain[0] = SCREEN_H * 0.55f;
	int bgslope = GetRandomValue(-2, 2);
	for (int i = 1; i < SCREEN_W; i++)
	{
		bgTerrain[i] = bgTerrain[i - 1] + bgslope;
		if (GetRandomValue(0, 100) < 20) bgslope = GetRandomValue(-2, 2);
		if (bgTerrain[i] <= BG_MIN) bgTerrain[i] = BG_MIN;
		if (bgTerrain[i] >= BG_MAX) bgTerrain[i] = BG_MAX;
	}
}

void Environment::Update(Player &gameP1, Player &gameP2, GamePlaySystem &gameplay, float deltaTime)
{
	// For now, only update sun cycle / explosion timers if needed
	if (explosion.active)
	{
		explosion.timer -= deltaTime;
		if (explosion.timer > 0.2f) explosion.radius += explosion.maxRadius * 8 * deltaTime;
		else explosion.radius -= explosion.maxRadius * 8 * deltaTime;
		if (explosion.timer <= 0) explosion.active = false;
	}
}

void Environment::Draw()
{
	// draw sky and sun
	if (sunisintheSky)
	{
		float daylength = 60.0f;
		float elapsed = GetTime() - cycleStartTime;
		float progress = elapsed / daylength;
		if (progress > 1.0f) progress = 1.0f;

		if (progress < 0.5f)
		{
			float skyBlend = progress / 0.5f;
			bgColor1.r = (unsigned char)(255 - (255 - 102) * skyBlend);
			bgColor1.g = (unsigned char)(109 + (191 - 109) * skyBlend);
			bgColor1.b = (unsigned char)(194 + (255 - 194) * skyBlend);
			bgColor1.a = 255;

			bgColor2.r = (unsigned char)(255 - (255 - 245) * skyBlend);
			bgColor2.g = (unsigned char)(161 + (245 - 161) * skyBlend);
			bgColor2.b = (unsigned char)(0 + (245 - 0) * skyBlend);
			bgColor2.a = 255;

			DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, bgColor1, bgColor2);
		}
		else if (progress > 0.5f)
		{
			float skyBlend = (progress - 0.5f) / 0.5f;
			bgColor1.r = (unsigned char)(102 - (102 - 0) * skyBlend);
			bgColor1.g = (unsigned char)(191 - (191 - 82) * skyBlend);
			bgColor1.b = (unsigned char)(255 - (255 - 172) * skyBlend);
			bgColor1.a = 255;

			bgColor2.r = (unsigned char)(245 + (255 - 245) * skyBlend);
			bgColor2.g = (unsigned char)(245 - (245 - 161) * skyBlend);
			bgColor2.b = (unsigned char)(245 - (245 - 0) * skyBlend);
			bgColor2.a = 255;

			DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, bgColor1, bgColor2);

		}

		float angle = PI - ((GetTime() - cycleStartTime) / 60.0f) * PI;
		float sunX = sunCenterX + sunHorizontalRadius * cos(angle);
		float sunY = sunHorizonY - sunVerticalRadius * sin(angle);
		Color sunColor = YELLOW;
		DrawCircle((int)sunX, (int)sunY, sunDiscRadius * 2.0f, Fade(sunColor, 0.1f));
		DrawCircle((int)sunX, (int)sunY, sunDiscRadius, sunColor);
		if ((GetTime() - cycleStartTime) / 60.0f >= 0.99f) { sunisintheSky = false; cycleStartTime = GetTime(); }
	}
	else
	{
		// night blending (simplified)
		float darkFactor = 0.5f;
		bgColor1 = Color{ (unsigned char)(30 * darkFactor), (unsigned char)(50 * darkFactor), (unsigned char)(90 * darkFactor), 255 };
		bgColor2 = Color{ (unsigned char)(60 * darkFactor), (unsigned char)(80 * darkFactor), (unsigned char)(120 * darkFactor), 255 };
		DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, bgColor1, bgColor2);
		if ((GetTime() - cycleStartTime) / 60.0f >= 0.99f) { sunisintheSky = true; cycleStartTime = GetTime(); }
	}

	// draw bg terrain
	for (int x = 0; x < SCREEN_W - 1; x++) DrawRectangle(x, (int)bgTerrain[x], 1, SCREEN_H - (int)bgTerrain[x], GRAY);

	// draw foreground terrain
	Color slightlyDarkBrown = ColorBrightness(BROWN, -0.3f);
	for (int x = 0; x < SCREEN_W - 1; x++)
	{
		DrawLine(x, terrain[x], x + 1, terrain[x + 1], GREEN);
		DrawRectangle(x, (int)terrain[x], 1, SCREEN_H - (int)terrain[x], slightlyDarkBrown);
		DrawRectangle(x, (int)terrain[x] + SCREEN_H * 0.05f, 1, SCREEN_H - (int)terrain[x] - SCREEN_H * 0.05f, DARKBROWN);
	}
}
