#include "common.h"

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "dark/style_dark.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "game.h"

// ----------------------------------------------------------------------------
// global state

global State state = {
    .window = {
        .width = 1280,
        .height = 720,
        .title = "Prong"
    },
    .exit_requested = false,
    .current_screen = TITLE,
};

global Assets assets = {0};

// ----------------------------------------------------------------------------
// entry point

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
// implementations

internal void Init() {
    // init raylib
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(state.window.width, state.window.height, state.window.title);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); // unbind ESC to exit

    // init raygui
    GuiLoadStyleDark();

    // init assets
    LoadAssets();

    // init game state
    state.render_texture = LoadRenderTexture(state.window.width, state.window.height);
    state.camera = (Camera2D){
        .target = (Vector2){0.0f, 0.0f},
        .offset = (Vector2){0.0f, 0.0f},
        .rotation = 0.0f,
        .zoom = 1.0f
    };

    state.entities.ball = (Ball){
        .pos = (Vector2){0, 100},
        .vel = (Vector2){0, 0},
        .anim = (Animation){
            .frame_count = 0,
            .frames_per_sec = 6,
            .textures = assets.ball_textures
        },
        .tex_index = 0,
        .radius = 25
    };

    const Vector2 paddle_size = {200, 50};
    state.entities.paddle = (Paddle){
        .rect = (Rectangle){
            -paddle_size.x / 2,
            (-state.window.height + paddle_size.y) / 2,
            paddle_size.x, paddle_size.y
        },
        .vel = (Vector2){0, 0},
        .anim = (Animation){
            .frame_count = 0,
            .frames_per_sec = 1,
            .textures = assets.paddle_textures
        },
        .tex_index = 0
    };
}

internal void Update() {
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

internal void UpdateGameplay() {
    const f32 dt = GetFrameTime();

    // handle input
    state.exit_requested = WindowShouldClose() || IsKeyPressed(KEY_ESCAPE);
    const bool left_down = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    const bool right_down = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // update animations
    Ball *ball = &state.entities.ball;
    ball->anim.frame_count++;
    if (ball->anim.frame_count >= (60 / ball->anim.frames_per_sec)) {
        ball->anim.frame_count = 0;
        ball->tex_index = (ball->tex_index + 1) % arrlen(ball->anim.textures);
    }

    Paddle *paddle = &state.entities.paddle;
    paddle->anim.frame_count++;
    if (paddle->anim.frame_count >= (60 / paddle->anim.frames_per_sec)) {
        paddle->anim.frame_count = 0;
        paddle->tex_index = (paddle->tex_index + 1) % arrlen(paddle->anim.textures);
    }

    // update entities
    ball->rect = (Rectangle){
            ball->pos.x - ball->radius,
            ball->pos.y - ball->radius,
            ball->radius * 2, ball->radius * 2};

    const f32 speed = 500;
    const i32 sign = left_down ? -1 : right_down ? 1 : 0;
    paddle->vel.x = sign * speed * dt;
    paddle->rect.x += paddle->vel.x;

    // constrain entities
    paddle->rect.x = Clamp(
            -state.window.width / 2,
            paddle->rect.x,
            (state.window.width / 2) - paddle->rect.width
    );

    // update camera
    state.camera.target = (Vector2){0, 0};
    state.camera.offset = (Vector2){state.window.width / 2, state.window.height / 2};
    state.camera.rotation = 0.0f;
    state.camera.zoom = 1.0f;
}

internal void DrawFrame() {
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
            Texture2D texture;
            Rectangle texture_rect;
            const Vector2 origin = {0, 0};

            const Ball ball = state.entities.ball;
            texture = ball.anim.textures[ball.tex_index];
            texture_rect = (Rectangle){0, 0, texture.width, texture.height};
            DrawTexturePro(texture, texture_rect, ball.rect, origin, 0.0f, WHITE);

            const Paddle paddle = state.entities.paddle;
            texture = paddle.anim.textures[paddle.tex_index];
            texture_rect = (Rectangle){0, 0, texture.width, texture.height};
            DrawTexturePro(texture, texture_rect, paddle.rect, origin, 0.0f, WHITE);
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

internal void Shutdown() {
    UnloadRenderTexture(state.render_texture);
    UnloadAssets();
    CloseWindow();
}

internal void LoadAssets() {
    arrput(assets.paddle_textures, LoadTexture("data/paddle-red.png"));

    for (int i = 0; i < 4; i++) {
        char path[64];
        sprintf_s(path, sizeof(path), "data/ball-red_%d.png", i);
        Texture2D texture = LoadTexture(path);
        arrput(assets.ball_textures, texture);
    }
}

internal void UnloadAssets() {
    for (int i = 0; i < arrlen(assets.paddle_textures); i++) {
        UnloadTexture(assets.paddle_textures[i]);
    }
    arrfree(assets.paddle_textures);

    for (int i = 0; i < arrlen(assets.ball_textures); i++) {
        UnloadTexture(assets.ball_textures[i]);
    }
    arrfree(assets.ball_textures);
}
