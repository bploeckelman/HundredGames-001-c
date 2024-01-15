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
    u64 frame_count;
    float frames_per_sec;
    Texture2D *textures;
} Animation;

// ----------------------------------------------------------------------------
// Game objects

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Rectangle rect;
    Animation anim;
    i32 tex_index;
    f32 radius;
} Ball;

typedef struct {
    Rectangle rect;
    Vector2 vel;
    Animation anim;
    i32 tex_index;
} Paddle;

// ----------------------------------------------------------------------------
// Game state data

typedef struct {
    struct Window {
        int width;
        int height;
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
