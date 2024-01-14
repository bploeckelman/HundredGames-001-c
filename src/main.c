#include <stdio.h>

#include "raylib.h"

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

    // init box2d
    InitPhysics(state.window);

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

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        // what's with these scales? I wouldn't have thought it'd take this much to get the ball moving
        const float vel_x = GetRandomValue(0, 1) == 0 ? -500.0f : 500.0f;
        const float vel_y = 1000.0f;

        b2Body_Wake(physics.bodies.ball);
        b2Body_SetLinearVelocity(physics.bodies.ball, (b2Vec2){vel_x, vel_y});
    }

    // update paddle velocity based on user input
    int dir = 0;
    if      (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dir = -1;
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir = 1;
    const float speed = 30.0f;
    state.paddle.vel.x = (dir == 0) ? 0 : state.paddle.vel.x + dir * speed;
    b2Body_SetLinearVelocity(physics.paddle.body, (b2Vec2){state.paddle.vel.x, 0});
    b2Body_SetAngularVelocity(physics.paddle.body, 0);

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
    ClearBackground(DARKGRAY);

    BeginMode2D(state.camera);
    switch (state.current_screen) {
        case TITLE: {
            DrawText("Prong", 10, 10, 40, LIGHTGRAY);

            const int button_width = 100;
            const int button_height = 50;
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
            const b2Vec2 ball_pos = b2Body_GetPosition(physics.ball.body);
            const b2Circle* ball = b2Shape_GetCircle(physics.ball.shape);
            DrawTexturePro(
                assets.ball_textures[0],
                (Rectangle){0, 0, assets.ball_textures[0].width, assets.ball_textures[0].height},
                (Rectangle){
                    ball_pos.x + ball->point.x - ball->radius,
                    ball_pos.y + ball->point.y - ball->radius,
                    ball->radius * 2, ball->radius * 2
                },
                (Vector2){0, 0},
                0.0f,
                WHITE);

            const b2Vec2 paddle_pos = b2Body_GetPosition(physics.paddle.body);
            const b2Polygon* paddle = b2Shape_GetPolygon(physics.paddle.shape);
            DrawTexturePro(
                assets.paddle_textures[0],
                (Rectangle){0, 0, assets.paddle_textures[0].width, assets.paddle_textures[0].height},
                (Rectangle){
                    paddle_pos.x + paddle->vertices[0].x,
                    paddle_pos.y + paddle->vertices[0].y,
                    paddle->vertices[2].x - paddle->vertices[0].x,
                    paddle->vertices[2].y - paddle->vertices[0].y
                },
                (Vector2){0, 0},
                0.0f,
                WHITE);

            DrawPhysicsDebug();
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

    for (int i = 0; i < 4; i++) {
        UnloadTexture(assets.ball_textures[i]);
    }
    UnloadTexture(assets.paddle_textures[0]);

    ClosePhysics();

    CloseWindow();
}
