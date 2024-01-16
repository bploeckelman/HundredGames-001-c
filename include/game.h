#ifndef PRONG_GAME_H
#define PRONG_GAME_H

#include "common.h"

#include "raylib.h"

// ----------------------------------------------------------------------------
// Game lifecycle functions

void Init();
void Update();
void UpdateGameplay();
void DrawFrame();
void Shutdown();

// ----------------------------------------------------------------------------
// Game screens

typedef enum GameScreen {
    TITLE = 0,
    GAMEPLAY,
    CREDITS,
} GameScreen;

// ----------------------------------------------------------------------------
// Helpers

typedef struct {
    u32 frames_per_sec;
    u32 frames_elapsed;
    i32 current_texture;
    Texture2D *textures;
} Animation;

Animation LoadAnimation(u32 frames_per_sec, Texture2D *textures);
Texture2D GetAnimationKeyframe(Animation anim);
void UpdateAnimation(Animation *anim);

// ----------------------------------------------------------------------------
// Game objects

typedef struct {
    f32 radius;
    Vector2 pos;
    Vector2 vel;
    Rectangle rect;
    Animation anim;
} Ball;

typedef struct {
    Vector2 vel;
    Rectangle rect;
    Animation anim;
} Paddle;

// ----------------------------------------------------------------------------
// Game state data

typedef struct {
    struct Window {
        i32 target_fps;
        i32 width;
        i32 height;
        char* title;
    } window;

    struct Entities {
        Ball ball;
        Paddle paddle;
    } entities;

    bool exit_requested;
    GameScreen current_screen;

    Camera2D camera;
    RenderTexture render_texture;
} State;

// ----------------------------------------------------------------------------
// Assets

typedef struct {
    Texture2D *ball_textures ;
    Texture2D *paddle_textures;
} Assets;

void LoadAssets();
void UnloadAssets();

#endif //PRONG_GAME_H
