#include <stdio.h>

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "dark/style_dark.h"

#include "common.h"

static State state = {
    .window = {
        .width = 1280,
        .height = 720,
        .title = "Prong"
    },
    .exit_requested = false,
    .current_screen = TITLE,
};

static Assets assets = {0};

// ----------------------------------------------------------------------------

static void Init();

static void Update();

static void UpdateGameplay();

static void DrawFrame();

static void Shutdown();

// ----------------------------------------------------------------------------

int main() {
    Init();
    while (!state.exit_requested) {
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
    SetExitKey(KEY_NULL); // unbind ESC to exit

    // init raygui
    GuiLoadStyleDark();

    // init assets
    for (int i = 0; i < 4; i++) {
        char path[64];
        sprintf_s(path, sizeof(path), "data/ball-red_%d.png", i);
        assets.ball_textures[i] = LoadTexture(path);
    }
    assets.paddle_textures[0] = LoadTexture("data/paddle-red.png");

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
    switch (state.current_screen) {
        case TITLE: {
            if (IsKeyReleased(KEY_ENTER) ||
                IsKeyReleased(KEY_SPACE) ||
                IsGestureDetected(GESTURE_TAP)) {
                state.current_screen = GAMEPLAY;
            }
            break;
        }
        case GAMEPLAY: {
            UpdateGameplay();
            break;
        }
        case CREDITS: {
            if (IsKeyReleased(KEY_ENTER) ||
                IsKeyReleased(KEY_SPACE)) {
                state.current_screen = TITLE;
            }
            break;
        }
    }
}

static void UpdateGameplay() {
    // handle input
    state.exit_requested = WindowShouldClose() || IsKeyPressed(KEY_ESCAPE);

    // update camera
    state.camera.target = (Vector2){0, 0};
    state.camera.offset = (Vector2){state.window.width / 2, state.window.height / 2};
    state.camera.rotation = 0.0f;
    state.camera.zoom = 1.0f;
}

static void DrawFrame() {
    // draw world to render texture
    BeginTextureMode(state.render_texture);
    ClearBackground(DARKGRAY);

    BeginMode2D(state.camera);
    switch (state.current_screen) {
        case TITLE: {
            DrawText("Prong", 10, 10, 40, LIGHTGRAY);

            const float button_width = 100;
            const float button_height = 50;
            const Rectangle button_rect = {
                (state.window.width - button_width) / 2,
                (state.window.height - button_height) / 2,
                button_width, button_height
            };
            if (GuiLabelButton(button_rect, GuiIconText(ICON_PLAYER_PLAY, "Play"))) {
                state.current_screen = GAMEPLAY;
            }
            break;
        }
        case GAMEPLAY: {
            const Vector2 ball_pos = {0, 100};
            const float ball_radius = 25;
            DrawTexturePro(
                assets.ball_textures[0],
                (Rectangle){0, 0, assets.ball_textures[0].width, assets.ball_textures[0].height},
                (Rectangle){
                    ball_pos.x - ball_radius,
                    ball_pos.y - ball_radius,
                    ball_radius * 2, ball_radius * 2
                },
                (Vector2){0, 0},
                0.0f,
                WHITE);

            Vector2 paddle_size = {assets.paddle_textures[0].width, assets.paddle_textures[0].height};
            DrawTexturePro(
                assets.paddle_textures[0],
                (Rectangle){0, 0, paddle_size.x, paddle_size.y},
                (Rectangle){-paddle_size.x / 2, -state.window.height / 2, paddle_size.x, paddle_size.y},
                (Vector2){0, 0},
                0.0f,
                WHITE);
            break;
        }
        case CREDITS: {
            DrawText("Credits", 10, 10, 40, LIGHTGRAY);
            break;
        }
    }
    EndMode2D();
    EndTextureMode();

    // draw render texture to screen
    BeginDrawing();
    ClearBackground(BLACK);

    // TODO - need to sort this out, I think its the UI that is flipped, gamescreen stuff isn't
    const int flip_y = state.current_screen == GAMEPLAY ? 1 : -1;
    DrawTexturePro(
        state.render_texture.texture,
        (Rectangle){0, 0, state.render_texture.texture.width, flip_y * state.render_texture.texture.height},
        (Rectangle){0, 0, state.window.width, state.window.height},
        (Vector2){0, 0},
        0.0f,
        WHITE);
    EndDrawing();
}

static void Shutdown() {
    UnloadRenderTexture(state.render_texture);

    UnloadTexture(assets.paddle_textures[0]);
    for (int i = 0; i < 4; i++) {
        UnloadTexture(assets.ball_textures[i]);
    }

    CloseWindow();
}
