#include "raylib.h"
#include<string>
#include <filesystem>
#include"GameplaySystem.h"
#include "environment.h"
#include "cannon.h"

enum class GameState
{
    StartMenu,
    Transition,
    Playing,
    GameOver
};

int main()
{

    GameState gameState = GameState::StartMenu;

    //-----------------------For Transition------------------------------------------------
    constexpr int STRIP_COUNT = 16;

    constexpr float TRANSITION_TIME = 1.5f;

    constexpr float BANNER_TIME = 1.2f;

    bool bannerDone = false;

    int winner = 0;
    double transitionStartTime=0.0f;

    int colorchanger = 1;

    Player player1;
    Player player2;
    GamePlaySystem gameplay;

    // Demo player for the integrated try24 gameplay (position/angle/power + Cannon)
    struct DemoPlayer {
        float x=0, y=0;
        float angle=45.0f;
        float power=300.0f;
        int hp=3;
        int coins=0;
        float explosionRadius=30;
        Cannon cannon;
    } d1, d2;

    struct Projectile { float x=0,y=0,vx=0,vy=0; bool active=false; int owner=0; } proj;
    struct Explosion { float x=0,y=0,radius=0,timer=0; bool active=false; float maxRadius=30; } explosion;

    // smoke trail particles for cannonball
    struct Smoke { float x; float y; float life; };
    std::vector<Smoke> smokeParticles;

    Environment env;

    bool shopOpen = false;
    InitWindow(800, 600, "Practice");


    bool landedhitonGround = false;

    // initialize audio device before using audio
    InitAudioDevice();

    ToggleFullscreen();





    int SCREEN_WIDTH = GetScreenWidth();
    int SCREEN_HEIGHT = GetScreenHeight();
    env.Init(SCREEN_WIDTH, SCREEN_HEIGHT);
    // initialize demo cannon players on terrain
    d1.x = (float)GetRandomValue(50, 200);
    d2.x = (float)GetRandomValue(600, 750);
    d1.y = env.GetTerrainHeight((int)d1.x);
    d2.y = env.GetTerrainHeight((int)d2.x);
    // sync Gameplay players' cannon visuals with demo positions
    player1.posX = d1.x; player1.posY = d1.y; player1.angle = d1.angle; player1.power = d1.power; player1.explosionRadius = d1.explosionRadius;
    player2.posX = d2.x; player2.posY = d2.y; player2.angle = d2.angle; player2.power = d2.power; player2.explosionRadius = d2.explosionRadius;
    player1.cannon.groundPos = { player1.posX, player1.posY };
    player2.cannon.groundPos = { player2.posX, player2.posY };
    player1.cannon.playerIndex = 0; player2.cannon.playerIndex = 1;
    player1.cannon.active = true; player2.cannon.active = true;

    // helper to handle end-of-turn logic: switch turn, decrement buffs, randomize wind
    auto EndTurn = [&](void) {
        gameplay.switchTurn();
        // decrement turn-based buffs
        if (player1.damageBoostTurns > 0) --player1.damageBoostTurns;
        if (player2.damageBoostTurns > 0) --player2.damageBoostTurns;
        if (player1.windShieldTurns > 0) --player1.windShieldTurns;
        if (player2.windShieldTurns > 0) --player2.windShieldTurns;

        // randomize wind gradually
        int delta = GetRandomValue(-60, 60);
        float newWind = env.wind + (float)delta;
        if (newWind > 150.0f) newWind = 150.0f;
        if (newWind < -150.0f) newWind = -150.0f;
        // small chance to flip sign
        if (GetRandomValue(0, 100) < 10) newWind = -newWind;
        env.wind = newWind;
    };
    player1.cannon.playerIndex = 0; player2.cannon.playerIndex = 1;
    player1.cannon.bodyColor = { 60, 110, 200, 255 };
    player1.cannon.barrelColor = { 40, 70, 140, 255 };
    player2.cannon.bodyColor = { 200, 70, 60, 255 };
    player2.cannon.barrelColor = { 140, 45, 40, 255 };
    // initialize gameplay player health and cannon health mapping (start at max health)
    player1.health = player1.maxHealth; player2.health = player2.maxHealth;
    player1.cannon.health = player1.health; player2.cannon.health = player2.health;

    // projectile physics parameters
    const float GRAVITY = 200.0f;
    // Direct hit detection radius (center-to-center). Use a stricter threshold
    // so incidental ground impacts don't count as direct hits. We require the
    // projectile center to be within 60% of the original hit radius to
    // consider it a direct hit (i.e., at least 40% overlap of the hit radius).
    constexpr float DIRECT_HIT_RADIUS = 27.0f;
    constexpr float DIRECT_HIT_THRESHOLD = DIRECT_HIT_RADIUS * 0.6f; // 60% of radius
    float icon_x = SCREEN_WIDTH - 100;
    float icon_y = 80;
    int shopwindowthickness = 10;
    const float insideshopicon_x = 0.35f * SCREEN_WIDTH;
    const float insideshopicon_y = 0.325 * SCREEN_HEIGHT;

    int coins = 0;
    



    //-------------------------------For StartMenu--------------------------------------

    double titleAnimationStart = 0.0;
    bool titleAnimationFinished = false;

    const float cannFinalX = SCREEN_WIDTH / 2.0f - 280.0f;
    const float nFinalX = SCREEN_WIDTH / 2.0f + 112.0f;

   float cannX = -500.0f;              // Start off-screen left
    float nX = SCREEN_WIDTH + 500.0f; // Start off-screen right

    //for cannon ball
    float ballScaleX = 1.0f;
    float ballScaleY = 1.0f;

    bool ballImpact = false;
    double ballImpactStart = 0.0;

    // for buttons
    float startButtonAlpha = 0.0f;
    float exitButtonAlpha = 0.0f;

    titleAnimationStart = -1;


    // ---------------------LOADING ASSETS---------------------
    Texture2D shopIcon = LoadTexture("assets/store.png");
    Texture2D minishopIcon = LoadTexture("assets/smallshop.png");
    Texture2D buyButton = LoadTexture("assets/buy.png");
    Texture2D startbutton = LoadTexture("assets/startbutton.png");
    Texture2D exitbutton = LoadTexture("assets/exitbutton.png");
    Texture2D cannonball = LoadTexture("assets/cannonball.png");
    Texture2D gameover = LoadTexture("assets/gameovericon.png");
    Texture2D CANN = LoadTexture("assets/CANN.png");
    Texture2D N = LoadTexture("assets/N.png");


    //------------LOADING AUDIO ASSETS----------------
    Sound coinSound = LoadSound("resources/coinaquired.wav");
    Sound transition2 = LoadSound("resources/transition2.wav");
    Sound shootsound = LoadSound("resources/cannonshoot.wav");
    Sound shopclick = LoadSound("resources/shopclick.wav");
    Sound itembought = LoadSound("resources/itembought.wav");
    Sound cannonhit = LoadSound("resources/cannonhit.wav");
    Sound gameOverSound = LoadSound("resources/levelcompleted.wav");
    Sound CannonsquashSound = LoadSound("resources/Startmenu.wav");
	Sound menubutton = LoadSound("resources/startexitbutton.wav");

    Vector2 shopPosition = { icon_x,icon_y };
    Vector2 shopIconposition = { insideshopicon_x, insideshopicon_y };
    Vector2 startbuttonPosition = { SCREEN_WIDTH / 2.0f - 125, SCREEN_HEIGHT / 2.0f -40 };
    Vector2 exitbuttonPosition = { SCREEN_WIDTH / 2.0f - 125,SCREEN_HEIGHT / 2.0f +80 };
 


    Rectangle shopButton =
    {
        icon_x,
        icon_y,
        64,
        64
    };
    Rectangle startButtonRect =
    {
    startbuttonPosition.x+30,
    startbuttonPosition.y+20,
    190,
    70
    };
    Rectangle exitButtonRect =
    {
        exitbuttonPosition.x+30,
        exitbuttonPosition.y+20,
        190,
        60
    };


    const int lineSpacing = 40;
    float textofButton_x = insideshopicon_x;
    float textofButton_y = insideshopicon_y + 50;

    const char* boosts[4] = { "DAMAGE BOOST   Cost: 1 Coin","BOOST RADIUS   Cost: 2 Coins","WIND SHEILD   Cost: 1 Coin","REPAIR KIT   Cost: 1 Coin" };

    Rectangle outershopWindow = { 0.3f * SCREEN_WIDTH, 0.3f * SCREEN_HEIGHT, 0.4f * SCREEN_WIDTH, 0.5f * SCREEN_HEIGHT };
    Rectangle innershopWindow = { 0.3f * SCREEN_WIDTH + shopwindowthickness, 0.3f * SCREEN_HEIGHT + shopwindowthickness, 0.4f * SCREEN_WIDTH - shopwindowthickness * 2, 0.5f * SCREEN_HEIGHT - shopwindowthickness * 2 };

    while (!WindowShouldClose())
    {
       

        Vector2 mouse = GetMousePosition();

       

        BeginDrawing();
        ClearBackground(DARKGRAY);

       //-------------------------------------------------------START MENU-----------------------------------------------------------------------------------------------
        if (gameState == GameState::StartMenu)
        {
            

            if (titleAnimationStart < 0.0)
            {
                titleAnimationStart = GetTime();
            }

            float t = GetTime() - titleAnimationStart;

            if (!titleAnimationFinished)
            {
                // ---------- CANN ----------
                if (t < 1.20f)
                {
                    // Fly in fast, slow near impact
                    float p = t / 1.20f;

                    // Ease-Out Cubic
                    p = 1.0f - powf(1.0f - p, 3.0f);

                    cannX = -500 + (cannFinalX + 8 + 500) * p;
                }
                else if (t < 1.70f)
                {
                    // Bounce back
                    float p = (t - 1.20f) / 0.50f;

                    cannX = (cannFinalX + 8) + (cannFinalX - 15 - (cannFinalX + 8)) * p;
                }
                else if (t < 2.20f)
                {
                    // Settle
                    float p = (t - 1.70f) / 0.50f;

                    cannX = (cannFinalX - 15) + (cannFinalX - (cannFinalX - 15)) * p;
                }
                else
                {
                    cannX = cannFinalX;
                    // ---------------- N Animation ----------------
                    if (t < 3.30f)
                    {
                        float nt = (t - 2.20f) / 1.10f;

                        if (nt < 0.0f)
                            nt = 0.0f;

                        if (nt > 1.0f)
                            nt = 1.0f;

                        // Ease-out cubic
                        nt = 1.0f - powf(1.0f - nt, 3.0f);

                        if (nt < 0.85f)
                        {
                            nX = SCREEN_WIDTH + 500.0f -
                                (SCREEN_WIDTH + 500.0f - nFinalX) * (nt / 0.85f);
                        }
                        else
                        {
                            float bounce = (nt - 0.85f) / 0.15f;

                            if (!ballImpact)
                            {
                                ballImpact = true;
                                ballImpactStart = GetTime();
                            }
                            
                            nX = nFinalX - sinf(bounce * PI) * 14.0f;
                            PlaySound(CannonsquashSound);
                        }
                    }
                    else
                    {
              
                        nX = nFinalX;
                        
                        
                        titleAnimationFinished = true;
                    }
                }
            }

            /* First part of CANNON i.e.CANN
            DrawText("2D CANN", SCREEN_WIDTH / 2 - 180, 150, 50, WHITE);
            */
            
            DrawTextureEx(
                CANN,
                { cannX, 150.0f },
                0.0f,
                0.30f,
                WHITE
            );


            if(ballImpact)
            {
                float bt = GetTime() - ballImpactStart;

                if (bt < 0.35f)
                {
                    // Stretch vertically
                    ballScaleX = 0.88f;
                    ballScaleY = 1.18f;
                }
                else if (bt < 0.65f)
                {
                    // Squash horizontally
                    ballScaleX = 1.18f;
                    ballScaleY = 0.88f;
                }
                else
                {
                    // Back to normal
                    ballScaleX = 1.0f;
                    ballScaleY = 1.0f;
                    ballImpact = false;
                }
            }



            // ---------------- Cannonball ----------------

            const float ballX = SCREEN_WIDTH / 2.0f + 41.0f;
            const float ballY = 153.0f;
            const float ballScale = 0.22f;

            Rectangle source =
            {
                0.0f,
                0.0f,
                (float)cannonball.width,
                (float)cannonball.height
            };

            Rectangle dest =
            {
                ballX + (cannonball.width * ballScale) / 2.0f,
                ballY + (cannonball.height * ballScale) / 2.0f,
                cannonball.width * ballScale * ballScaleX,
                cannonball.height * ballScale * ballScaleY
            };

            Vector2 origin =
            {
                dest.width / 2.0f,
                dest.height / 2.0f
            };

            DrawTexturePro(
                cannonball,
                source,
                dest,
                origin,
                0.0f,
                WHITE
            );

            DrawTextureEx(
                N,
                { nX,154.0f },
                0.0f,
                0.16f,
                WHITE
            );


            //-----------For Start and Exit Buttons---------------
            if (titleAnimationFinished)
            {
                if (startButtonAlpha < 255.0f)
                    startButtonAlpha += 250.0f * GetFrameTime();

                if (startButtonAlpha > 255.0f)
                    startButtonAlpha = 255.0f;

                if (startButtonAlpha >= 255.0f)
                {
                    if (exitButtonAlpha < 255.0f)
                        exitButtonAlpha += 250.0f * GetFrameTime();

                    if (exitButtonAlpha > 255.0f)
                        exitButtonAlpha = 255.0f;
                }
            }


            if (titleAnimationFinished)
            {

                if (CheckCollisionPointRec(mouse, startButtonRect))
                {
                    Color startColor = WHITE;
                    startColor.a = (unsigned char)startButtonAlpha;

                    DrawTextureEx(
                        startbutton,
                        startbuttonPosition,
                        0,
                        0.5f,
                        startColor
                    );

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
						PlaySound(menubutton);
                        transitionStartTime = GetTime();



                        gameState = GameState::Transition;
                    }
                }
                else
                {
                    DrawRectangleRounded(startButtonRect, 0.5, 10, WHITE);

                    Color startColor = WHITE;
                    startColor.a = (unsigned char)startButtonAlpha;

                    DrawTextureEx(
                        startbutton,
                        startbuttonPosition,
                        0,
                        0.45f,
                        startColor
                    );
                }



                if (CheckCollisionPointRec(mouse, exitButtonRect))
                {
                    Color exitColor = WHITE;
                    exitColor.a = (unsigned char)exitButtonAlpha;

                    DrawTextureEx(
                        exitbutton,
                        exitbuttonPosition,
                        0,
                        0.37f,
                        exitColor
                    );

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
						PlaySound(menubutton);
                        CloseWindow();
                        return 0;
                    }
                }
                else
                {
                    DrawRectangleRounded(exitButtonRect, 0.5, 10, WHITE);
                    Color exitColor = WHITE;
                    exitColor.a = (unsigned char)exitButtonAlpha;

                    DrawTextureEx(
                        exitbutton,
                        exitbuttonPosition,
                        0,
                        0.34f,
                        exitColor
                    );

                    
                }
            }

        }

        //-----------------------------------------------TRANSITION --------------------------------------------------------------------
        else if (gameState == GameState::Transition)
        {
            PlaySound(transition2);
            // Update and draw environment so the scene is visible behind the transition strips
            float dt_env = GetFrameTime();
            env.Update(player1, player2, gameplay, dt_env);
            env.Draw();

            float elapsed = GetTime() - transitionStartTime;

            float stripHeight = (float)SCREEN_HEIGHT / STRIP_COUNT;

            for (int i = 0; i < STRIP_COUNT; i++)
            {

                float bannerElapsed = elapsed - TRANSITION_TIME;

                if (bannerElapsed > 0.0f)
                {
                    float bannerWidth = SCREEN_WIDTH * 0.7f;
                    float bannerHeight = 70;

                    float x;

                    if (bannerElapsed < 0.2f)
                    {
                        // Slide in
                        x = -bannerWidth + (bannerElapsed / 0.2f) * ((SCREEN_WIDTH - bannerWidth) / 2 + bannerWidth);
                    }
                    else if (bannerElapsed < 1.0f)
                    {
                        // Stay
                        x = (SCREEN_WIDTH - bannerWidth) / 2;
                    }
                    else
                    {
                        // Slide out
                        float t = (bannerElapsed - 1.0f) / 0.2f;

                        if (t > 1.0f)
                            t = 1.0f;

                        x = (SCREEN_WIDTH - bannerWidth) / 2 + t * (SCREEN_WIDTH);
                    }
                    DrawRectangleRounded(
                        { x, SCREEN_HEIGHT / 2.0f - bannerHeight / 2.0f,
                          bannerWidth, bannerHeight },
                        0.25f,
                        10,
                        BLACK
                    );

                    DrawText(
                        "COMMENCE FIRE",
                        (int)(x + bannerWidth / 2 - MeasureText("COMMENCE FIRE", 36) / 2),
                        SCREEN_HEIGHT / 2 - 18,
                        36,
                        WHITE
                    );
                }

                float progress = (elapsed - i * 0.02f) / TRANSITION_TIME;

                if (progress < 0.0f)
                    progress = 0.0f;

                if (progress > 1.0f)
                    progress = 1.0f;

                // Smooth easing
                progress = progress * progress * (3.0f - 2.0f * progress);

                float halfWidth = (SCREEN_WIDTH / 2.0f) * (1.0f - progress);

                // Left half
                if (i%2==0)
                {
                    DrawRectangle(
                        0,
                        (int)(i * stripHeight),
                        (int)halfWidth,
                        (int)stripHeight + 1,
                        { 125, 30, 32, 255 }
                    );
                }
                else
                {

                    DrawRectangle(
                        0,
                        (int)(i* stripHeight),
                        (int)halfWidth,
                        (int)stripHeight + 1,
                        { 175, 30, 32, 255 }
                    );
                }
                // Right half
                if (i%2==0)
                {
                    DrawRectangle(
                        SCREEN_WIDTH - (int)halfWidth,
                        (int)(i * stripHeight),
                        (int)halfWidth,
                        (int)stripHeight + 1,
                        { 125, 30, 32, 255 }
                    );
                }
                else
                {
                    DrawRectangle(
                        SCREEN_WIDTH - (int)halfWidth,
                        (int)(i * stripHeight),
                        (int)halfWidth,
                        (int)stripHeight + 1,
                        { 175, 30, 32, 255 }
                    );
                }

            }

            // do not draw cannons during transition (they should appear only in Playing state)

            if (elapsed >= TRANSITION_TIME+BANNER_TIME)
            {
                gameState = GameState::Playing;
            }
        }

        //----------------------------------------------PLAYING-------------------------------------------------------------------------
        else if (gameState == GameState::Playing)
        {

            // Update environment and draw behind UI/gameplay
            float dt = GetFrameTime();
            env.Update(player1, player2, gameplay, dt);
            env.Draw();

            // draw cannons in playing state
            player1.cannon.Draw();
            player2.cannon.Draw();

            // ensure gameplay players follow terrain in playing state
            player1.posY = env.GetTerrainHeight((int)player1.posX);
            player2.posY = env.GetTerrainHeight((int)player2.posX);
            player1.cannon.groundPos = { player1.posX, player1.posY };
            player2.cannon.groundPos = { player2.posX, player2.posY };

            // Handle input only in Playing state for current player (rotate and fire)
            Player* shooterP = (gameplay.getCurrentTurn() == 1) ? &player1 : &player2;
            Player* targetP = (gameplay.getCurrentTurn() == 1) ? &player2 : &player1;

            if (IsKeyDown(KEY_D)) shooterP->power += 100.0f * dt;
            if (IsKeyDown(KEY_A)) shooterP->power -= 100.0f * dt;
            if (IsKeyDown(KEY_W)) shooterP->angle += 60.0f * dt;
            if (IsKeyDown(KEY_S)) shooterP->angle -= 60.0f * dt;
            if (shooterP->angle < 0) shooterP->angle = 0;
            if (shooterP->angle > 90) shooterP->angle = 90;
            // map relative angle to cannon angleDeg
            if (shooterP == &player1) shooterP->cannon.angleDeg = shooterP->angle; else shooterP->cannon.angleDeg = 180.0f - shooterP->angle;
            shooterP->cannon.power = shooterP->power;

            // Fire
            if (IsKeyPressed(KEY_SPACE) && !proj.active)
            {
                // spawn projectile at muzzle
				PlaySound(shootsound);
                Vector2 muzzle = shooterP->cannon.GetMuzzlePos();
                proj.x = muzzle.x; proj.y = muzzle.y;
                float rad = shooterP->cannon.angleDeg * (3.14159265f / 180.0f);
                proj.vx = shooterP->power * cosf(rad);
                proj.vy = -shooterP->power * sinf(rad);
                proj.active = true;
                proj.owner = (gameplay.getCurrentTurn()==1)?1:2;
                // spawn initial smoke (short lifetime)
                smokeParticles.push_back({proj.x, proj.y, 0.2f});
            }

            // simulate projectile
            if (proj.active)
            {
                proj.x += proj.vx * dt;
                proj.y += proj.vy * dt;
                proj.vy += GRAVITY * dt;
                // apply wind; reduce effect if shooter had an active wind shield when firing
                float windEffect = env.wind;
                if (proj.owner == 1 && player1.windShieldTurns > 0) windEffect *= 0.5f;
                if (proj.owner == 2 && player2.windShieldTurns > 0) windEffect *= 0.5f;
                proj.vx += windEffect * dt;
                // spawn smoke trail periodically
                if (GetRandomValue(0, 100) < 12) {
                    smokeParticles.push_back({proj.x, proj.y, 0.225f});
                }

                // collision with target
                float dx = proj.x - targetP->posX;
                float dy = proj.y - targetP->posY;
                float dist = sqrtf(dx*dx + dy*dy);
                if (dist <= 27.0f)
                {
                    PlaySound(cannonhit);

                    // hit
                    // determine damage (damage boost applies to direct hits)
                    int damageToApply = NORMAL_DAMAGE;
                    if (shooterP->damageBoostTurns > 0)
                    {
                        damageToApply = BOOST_DAMAGE;
                    }

                    gameplay.applyDamage(*targetP, damageToApply);
                    // award a coin for any direct hit
                    
                        PlaySound(coinSound);
                    
                    gameplay.awardCoin(*shooterP);
                    
                    // create explosion
                    explosion.x = proj.x; explosion.y = proj.y; explosion.timer = 0.4f; explosion.active = true; explosion.maxRadius = shooterP->explosionRadius; explosion.radius = 0;
                    // apply crater to terrain (explosion.maxRadius already reflects any BlastRadius upgrade)
                    env.ApplyExplosion(explosion.x, explosion.y, explosion.maxRadius);
                    // end turn with wind randomization (EndTurn performs the switch)
                    EndTurn();

                    proj.active = false;
                }
                else if (proj.x < 0 || proj.x > SCREEN_WIDTH || proj.y > SCREEN_HEIGHT)
                {
                    proj.active = false;
                    gameplay.switchTurn();
                }
                else
                {
                    // ground collision
                    float groundY = env.GetTerrainHeight((int)proj.x);
                   // landedhitonGround = true;
                    if (proj.y >= groundY)
                    {
                        explosion.x = proj.x; explosion.y = proj.y; explosion.timer = 0.4f; explosion.active = true; explosion.maxRadius = shooterP->explosionRadius; explosion.radius = 0;
                        // apply crater to terrain (explosion.maxRadius already reflects any BlastRadius upgrade)
                        env.ApplyExplosion(explosion.x, explosion.y, explosion.maxRadius);

                        // end turn (EndTurn will switch turn and randomize wind)
                        EndTurn();
                        proj.active = false;

                        // area damage (apply side hit damage to players inside blast)
                        float d1dx = explosion.x - player1.posX; float d1dy = explosion.y - player1.posY; if (sqrtf(d1dx*d1dx + d1dy*d1dy) <= 50.0f) gameplay.applyDamage(player1, SIDEHIT_DAMAGE);
                        float d2dx = explosion.x - player2.posX; float d2dy = explosion.y - player2.posY; if (sqrtf(d2dx*d2dx + d2dy*d2dy) <= 50.0f) gameplay.applyDamage(player2, SIDEHIT_DAMAGE);
                    }
                }
               /* if (landedhitonGround)
                {
					gameplay.switchTurn();
                }*/
            }

            // update explosion
            if (explosion.active)
            {
                explosion.timer -= dt;
                if (explosion.timer > 0.2f) explosion.radius += explosion.maxRadius * 8.0f * dt;
                else explosion.radius -= explosion.maxRadius * 8.0f * dt;
                if (explosion.timer <= 0.0f) explosion.active = false;
            }

            // draw projectile (cannonball) if active
            if (proj.active)
            {
                // reaffirm draw call (no-op change to maintain patch consistency)
                DrawCircle((int)proj.x, (int)proj.y, 6, BLACK);
            }

            // draw explosion visual
            if (explosion.active)
            {
                DrawCircle((int)explosion.x, (int)explosion.y, (int)explosion.radius, ORANGE);
                DrawCircle((int)explosion.x, (int)explosion.y, (int)(explosion.radius*0.5f), YELLOW);
            }
            // update and draw smoke trail particles
            for (int i = (int)smokeParticles.size() - 1; i >= 0; --i)
            {
                // decrease life
                smokeParticles[i].life -= dt;
                if (smokeParticles[i].life <= 0.0f)
                {
                    // remove expired particle
                    smokeParticles.erase(smokeParticles.begin() + i);
                }
                else
                {
                    // slight upward drift
                    smokeParticles[i].y -= 10.0f * dt;

                    // draw particle with fading alpha and small size, use black color
                    float lifeNorm = smokeParticles[i].life / 0.45f; // normalize against typical spawn life
                    if (lifeNorm > 1.0f) lifeNorm = 1.0f;
                    if (lifeNorm < 0.0f) lifeNorm = 0.0f;
                    Color col = Fade(BLACK, lifeNorm);
                    float size = 1.5f + (1.0f - lifeNorm) * 2.0f; // small particles
                    DrawCircleV({ smokeParticles[i].x, smokeParticles[i].y }, size, col);
                }
            }

            Player* currentPlayer;

            // Color the turn text to match the active player's cannon color
            Color turnColor = (gameplay.getCurrentTurn() == 1) ? player1.cannon.bodyColor : player2.cannon.bodyColor;
            DrawText(TextFormat("PLAYER %d turn", gameplay.getCurrentTurn()), SCREEN_WIDTH / 2 - 90, 40, 30, turnColor);

            // draw health bars for both players (top-left and top-right)
            float barW = 160.0f;
            float barH = 18.0f;
            // Player 1 bar (top-left)
            float p1x = 20.0f;
            float p1y = 10.0f;
            // Use integer health values from GameplaySystem's Player struct
            int hp1 = player1.health;
            int max1 = player1.maxHealth;
            float pct1 = (max1 > 0) ? (float)hp1 / (float)max1 : 0.0f;
            if (pct1 < 0.0f) pct1 = 0.0f; if (pct1 > 1.0f) pct1 = 1.0f;
            DrawRectangle((int)p1x, (int)p1y, (int)barW, (int)barH, Fade(BLACK, 0.5f));
            DrawRectangle((int)p1x + 1, (int)p1y + 1, (int)((barW - 2) * pct1), (int)(barH - 2), GREEN);
            DrawText(TextFormat("P1 HP: %d/%d", hp1, max1), (int)p1x, (int)(p1y + barH + 2), 14, WHITE);

            // Player 2 bar (top-right)
            float p2x = SCREEN_WIDTH - 20.0f - barW;
            float p2y = 10.0f;
            int hp2 = player2.health;
            int max2 = player2.maxHealth;
            float pct2 = (max2 > 0) ? (float)hp2 / (float)max2 : 0.0f;
            if (pct2 < 0.0f) pct2 = 0.0f; if (pct2 > 1.0f) pct2 = 1.0f;
            DrawRectangle((int)p2x, (int)p2y, (int)barW, (int)barH, Fade(BLACK, 0.5f));
            DrawRectangle((int)p2x + 1, (int)p2y + 1, (int)((barW - 2) * pct2), (int)(barH - 2), GREEN);
            DrawText(TextFormat("P2 HP: %d/%d", hp2, max2), (int)(p2x + barW - 110), (int)(p2y + barH + 2), 14, WHITE);

            // display power and wind under the health bars (show active player's power)
            Player* active = (gameplay.getCurrentTurn() == 1) ? &player1 : &player2;
            DrawText(TextFormat("Power: %.0f", active->power), 20, (int)(p1y + barH + 22), 20, RED);
            DrawText(TextFormat("WIND: %.1f", env.wind), 20, (int)(p1y + barH + 42), 20, GREEN);

            if (gameplay.getCurrentTurn() == 1)
            {
                currentPlayer = &player1;
            }
            else
            {
                currentPlayer = &player2;
            }
            /*For testing purpose*/
            if (IsKeyPressed(KEY_T))
            {
                gameplay.awardCoin(*currentPlayer);
            }
            
            // noop placeholder: keep edit logs consistent (no functional change)

            if (!shopOpen)//--------------------SHOP ICON------------------
            {
                if (IsKeyPressed(KEY_E))
                {
					PlaySound(shopclick);
                    shopOpen = true;
                }
                if (CheckCollisionPointRec(mouse, shopButton))
                {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
						PlaySound(shopclick);
                        shopOpen = true;

                    }
                    else
                    {
                        DrawTextureEx(shopIcon, shopPosition, 0, 0.14f, WHITE);
                    }
                }
                else
                {
                    DrawTextureEx(shopIcon, shopPosition, 0, 0.12f, WHITE);
                }
                // DrawText("OPEN UPGRADABLES", 0.41f * SCREEN_WIDTH, 0.45f * SCREEN_HEIGHT, 5, GOLD);
            }
            else   //------------UI OF INSIDE THE SHOP WINDOW--------------------
            {

                DrawRectangleRounded(outershopWindow, 0.3f, 20, LIGHTGRAY);
                DrawRectangleRoundedLines(outershopWindow, 0.3f, 20, BLACK);
                DrawRectangleRounded({ 0.3f * SCREEN_WIDTH + shopwindowthickness - 3, 0.3f * SCREEN_HEIGHT + shopwindowthickness + -3, 0.4f * SCREEN_WIDTH - shopwindowthickness * 2 + 6, 0.5f * SCREEN_HEIGHT - shopwindowthickness * 2 +6 }, 0.3f, 20, BLACK);

                DrawRectangleRounded(innershopWindow, 0.3f, 20, RAYWHITE);
                DrawRectangleRoundedLines(innershopWindow, 0.3f, 20, BLACK);

                DrawTextureEx(minishopIcon, shopIconposition, 0, 0.08f, WHITE);
                DrawText("UPGRADES", 0.4f * SCREEN_WIDTH, 0.35f * SCREEN_HEIGHT, 8, BLUE);
                DrawText(TextFormat("Coins: %i", currentPlayer->coins), 0.4f * SCREEN_WIDTH + 170, 0.35f * SCREEN_HEIGHT, 8, PURPLE);

                for (int i = 0;i < 4;i++)
                {
                    Color buttontextColor = WHITE;

                    Rectangle button =
                    {
                        insideshopicon_x + 160,
                        insideshopicon_y + 50 + i * lineSpacing,
                        80,
                        20
                    };

                    if (i == 0)
                    {
                        buttontextColor = RED;
                        DrawText(TextFormat("Left= %i", currentPlayer->damageBoostLeft), textofButton_x, textofButton_y + i * lineSpacing + 20, 10, BLACK);
                    }
                    else if (i == 1)
                    {
                        buttontextColor = ORANGE;
                        DrawText(TextFormat("Left= %i", currentPlayer->blastRadiusLeft), textofButton_x, textofButton_y + i * lineSpacing + 20, 10, BLACK);
                    }
                    else if (i == 2)
                    {
                        buttontextColor = BLUE;
                        DrawText(TextFormat("Left= %i", currentPlayer->windShieldLeft), textofButton_x, textofButton_y + i * lineSpacing + 20, 10, BLACK);
                    }
                    else if (i == 3)
                    {
                        buttontextColor = GREEN;
                        DrawText(TextFormat("Left= %i", currentPlayer->repairKitLeft), textofButton_x, textofButton_y + i * lineSpacing + 20, 10, BLACK);
                    }
                    DrawText(boosts[i], textofButton_x, textofButton_y + i * lineSpacing + 10, 10, buttontextColor);

                    DrawTextureEx(buyButton, { insideshopicon_x + 160,insideshopicon_y + 50 + i * lineSpacing }, 0, 0.3f, WHITE);
                    if (i == 0 && currentPlayer->coins >= DAMAGE_BOOST_COST && currentPlayer->damageBoostLeft > 0)
                    {

                        if (CheckCollisionPointRec(mouse, button))
                        {
                            DrawTextureEx(buyButton, { insideshopicon_x + 160,insideshopicon_y + 50 + i * lineSpacing }, 0, 0.3f, GOLD);


                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                            {
								PlaySound(itembought);
                                gameplay.buyUpgrade(*currentPlayer, UpgradeType::DamageBoost);
                            }
                        }
                    }

                    else if (i == 1 && currentPlayer->coins >= BLAST_COST && currentPlayer->blastRadiusLeft > 0)
                    {

                        if (CheckCollisionPointRec(mouse, button))
                        {
                            DrawTextureEx(buyButton, { insideshopicon_x + 160,insideshopicon_y + 50 + i * lineSpacing }, 0, 0.3f, GOLD);


                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                            {
								PlaySound(itembought);
                                gameplay.buyUpgrade(*currentPlayer, UpgradeType::BlastRadius);
                            }
                        }
                    }
                    else if (i == 2 && currentPlayer->coins >= WIND_SHIELD_COST && currentPlayer->windShieldLeft > 0)
                    {

                        if (CheckCollisionPointRec(mouse, button))
                        {
                            DrawTextureEx(buyButton, { insideshopicon_x + 160,insideshopicon_y + 50 + i * lineSpacing }, 0, 0.3f, GOLD);


                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                            {
								PlaySound(itembought);
                                gameplay.buyUpgrade(*currentPlayer, UpgradeType::WindShield);
                            }
                        }
                    }
                    else if (i == 3 && currentPlayer->coins >= REPAIR_KIT_COST && currentPlayer->repairKitLeft > 0 && currentPlayer->health < currentPlayer->maxHealth)
                    {

                        if (CheckCollisionPointRec(mouse, button))
                        {
                            DrawTextureEx(buyButton, { insideshopicon_x + 160,insideshopicon_y + 50 + i * lineSpacing }, 0, 0.3f, GOLD);


                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                            {

								PlaySound(itembought);
                                gameplay.buyUpgrade(*currentPlayer, UpgradeType::RepairKit);
                            }
                        }
                    }

                    else
                    {
                        DrawTextureEx(buyButton, { insideshopicon_x + 160,insideshopicon_y + 50 + i * lineSpacing }, 0, 0.3f, GRAY);
                    }

                    if (IsKeyPressed(KEY_E))
                    {

                        PlaySound(shopclick);
                        shopOpen = false;

                    }

                }

            }
            /*For testing purpose*/
            if (IsKeyPressed(KEY_Q))
            {
                currentPlayer->health = 0;
            }
            
            

            winner = gameplay.checkWinner(player1, player2);

            if (winner != 0)
            {
                gameState = GameState::GameOver;

                PlaySound(gameOverSound);
            }

            if (IsKeyPressed(KEY_R))
            {
				
                // reset gameplay state and players
                player1 = Player();
                player2 = Player();
                gameplay = GamePlaySystem();

                // re-generate terrain and re-place demo cannons so restart feels fresh
                env.Init(SCREEN_WIDTH, SCREEN_HEIGHT);
                // randomize demo positions on new terrain
                d1.x = (float)GetRandomValue(50, 200);
                d2.x = (float)GetRandomValue(600, 750);
                d1.y = env.GetTerrainHeight((int)d1.x);
                d2.y = env.GetTerrainHeight((int)d2.x);

                // sync Gameplay players with demo positions and defaults
                player1.posX = d1.x; player1.posY = d1.y; player1.angle = d1.angle; player1.power = d1.power; player1.explosionRadius = d1.explosionRadius;
                player2.posX = d2.x; player2.posY = d2.y; player2.angle = d2.angle; player2.power = d2.power; player2.explosionRadius = d2.explosionRadius;
                player1.cannon.groundPos = { player1.posX, player1.posY };
                player2.cannon.groundPos = { player2.posX, player2.posY };
                player1.cannon.playerIndex = 0; player2.cannon.playerIndex = 1;
                player1.cannon.active = true; player2.cannon.active = true;
                player1.cannon.bodyColor = { 60, 110, 200, 255 };
                player1.cannon.barrelColor = { 40, 70, 140, 255 };
                player2.cannon.bodyColor = { 200, 70, 60, 255 };
                player2.cannon.barrelColor = { 140, 45, 40, 255 };

				winner = 0;
				shopOpen = false;
				titleAnimationFinished = -1;
				gameState = GameState::StartMenu;
            }
        }

        //--------------------------------------------------------GAME OVER---------------------------------------------------------------
        else if (gameState == GameState::GameOver)
        {
           
            ClearBackground(BLACK);

            DrawTextureEx(gameover,{SCREEN_WIDTH / 2.0f - 180,SCREEN_HEIGHT / 2.0f -120 },0.0f,0.7f,WHITE);

            DrawText(TextFormat("PLAYER %d WINS!", winner),SCREEN_WIDTH / 2 - 170,SCREEN_HEIGHT / 2 - 60,50,GOLD);

            DrawText( "Press R to Restart", SCREEN_WIDTH / 2 - 120,SCREEN_HEIGHT / 2 + 20, 25, GREEN );

            DrawText( "Press ESC to Quit",  SCREEN_WIDTH / 2 - 110,  SCREEN_HEIGHT / 2 + 60,  20,RED);
        
            if (IsKeyPressed(KEY_R))
            {
              

                // reset gameplay state and players
                player1 = Player();
                player2 = Player();
                gameplay = GamePlaySystem();

                // re-generate terrain and re-place demo cannons so restart feels fresh
                env.Init(SCREEN_WIDTH, SCREEN_HEIGHT);
                // randomize demo positions on new terrain
                d1.x = (float)GetRandomValue(50, 200);
                d2.x = (float)GetRandomValue(600, 750);
                d1.y = env.GetTerrainHeight((int)d1.x);
                d2.y = env.GetTerrainHeight((int)d2.x);

                // sync Gameplay players with demo positions and defaults
                player1.posX = d1.x; player1.posY = d1.y; player1.angle = d1.angle; player1.power = d1.power; player1.explosionRadius = d1.explosionRadius;
                player2.posX = d2.x; player2.posY = d2.y; player2.angle = d2.angle; player2.power = d2.power; player2.explosionRadius = d2.explosionRadius;
                player1.cannon.groundPos = { player1.posX, player1.posY };
                player2.cannon.groundPos = { player2.posX, player2.posY };
                player1.cannon.playerIndex = 0; player2.cannon.playerIndex = 1;
                player1.cannon.active = true; player2.cannon.active = true;
                player1.cannon.bodyColor = { 60, 110, 200, 255 };
                player1.cannon.barrelColor = { 40, 70, 140, 255 };
                player2.cannon.bodyColor = { 200, 70, 60, 255 };
                player2.cannon.barrelColor = { 140, 45, 40, 255 };

                winner = 0;
                shopOpen = false;

                titleAnimationFinished = -1;

                gameState = GameState::StartMenu;
            }
        }


        EndDrawing();
    }
    UnloadTexture(shopIcon);
    UnloadTexture(buyButton);
    UnloadTexture(minishopIcon);
    UnloadTexture(startbutton);
    UnloadTexture(exitbutton);
	UnloadTexture(gameover);
	UnloadTexture(N);
	UnloadTexture(CANN);
	UnloadTexture(cannonball);



	UnloadSound(menubutton);
	UnloadSound(shootsound);
	UnloadSound(cannonhit); 
	UnloadSound(shopclick); 
	UnloadSound(itembought);
	UnloadSound(transition2);
	UnloadSound(gameOverSound);
	UnloadSound(coinSound);
	UnloadSound(CannonsquashSound);
    

	CloseAudioDevice();

    CloseWindow();
}
