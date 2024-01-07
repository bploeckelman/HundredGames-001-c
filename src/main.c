#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "dark/style_dark.h"

#include "common.h"

#define GLSL_VERSION 330

static State state = {
    .window = {
        .width = 1280,
        .height = 720,
        .title = "Prong"
    },
};

static void Init();

static void UpdateDrawFrame();

static void Shutdown();

// ----------------------------------------------------------------------------

int main() {
    Init();
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
    Shutdown();
    return 0;
}

// ----------------------------------------------------------------------------

static void Init() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(state.window.width, state.window.height, state.window.title);
    SetTargetFPS(60);

    GuiLoadStyleDark();

    state.camera = (Camera2D){
        .target = (Vector2){0.0f, 0.0f},
        .offset = (Vector2){0.0f, 0.0f},
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    state.render_texture = LoadRenderTexture(state.window.width, state.window.height);
}

static void UpdateDrawFrame() {
    // draw world to render texture
    BeginTextureMode(state.render_texture);
    ClearBackground(RAYWHITE);

    BeginMode2D(state.camera);
    const int diameter = 100;
    DrawCircle(
        state.window.width / 2,
        state.window.height / 2,
        diameter / 2,
        RED);
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

    CloseWindow();
}
