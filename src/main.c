#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "dark/style_dark.h"

#include "../include/common.h"
#include "../include/physics.h"

static State state = {
    .window = {
        .width = 1280,
        .height = 720,
        .title = "Prong"
    },
};

static void Init();

static void Update();

static void DrawFrame();

static void Shutdown();

// ----------------------------------------------------------------------------

int main() {
    Init();
    while (!WindowShouldClose()) {
        Update();
        DrawFrame();
    }
    Shutdown();
    return 0;
}

// ----------------------------------------------------------------------------

static void Init() {
    // init raylib
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(state.window.width, state.window.height, state.window.title);
    SetTargetFPS(60);

    // init raygui
    GuiLoadStyleDark();

    // init box2d
    InitPhysics(state.window);

    // init game state
    state.camera = (Camera2D){
        .target = (Vector2){0.0f, 0.0f},
        .offset = (Vector2){0.0f, 0.0f},
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    state.render_texture = LoadRenderTexture(state.window.width, state.window.height);
}

static void Update() {
    // handle input
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        // what's with these scales? I wouldn't have thought it'd take this much to get the ball moving
        const float vel_x = GetRandomValue(0, 1) == 0 ? -500.0f : 500.0f;
        const float vel_y = 1000.0f;

        b2Body_Wake(physics.bodies.ball);
        b2Body_SetLinearVelocity(physics.bodies.ball, (b2Vec2){vel_x, vel_y});
    }

    // update systems
    UpdatePhysics();

    // update camera
    state.camera.target = (Vector2){0, 0};
    state.camera.offset = (Vector2){state.window.width / 2, state.window.height / 2};
    state.camera.rotation = 0.0f;
    state.camera.zoom = 1.0f;
}

static void DrawFrame() {
    // draw world to render texture
    BeginTextureMode(state.render_texture);
    ClearBackground(GRAY);

    BeginMode2D(state.camera);
    DebugDrawPhysics(); // NOTE - just debug physics drawing for now
    EndMode2D();

    EndTextureMode();

    // draw render texture to screen
    BeginDrawing();
    ClearBackground(BLACK);

    DrawTextureEx(state.render_texture.texture, (Vector2){0, 0}, 0, 1, WHITE);
    DrawText(state.window.title, 10, 10, 20, DARKGRAY);

    EndDrawing();
}

static void Shutdown() {
    UnloadRenderTexture(state.render_texture);

    ClosePhysics();

    CloseWindow();
}
