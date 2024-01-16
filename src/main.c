#include "common.h"

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "dark/style_dark.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "game.h"

// ----------------------------------------------------------------------------
// Global data

global State state = {
    .window = {
        .target_fps = 60,
        .width = 1280,
        .height = 720,
        .title = "Prong"
    },
    .exit_requested = false,
    .current_screen = TITLE,
};

global Assets assets = {0};

// ----------------------------------------------------------------------------
// Entry point

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
// Lifecycle functions

internal void Init() {
    // init raylib
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(state.window.width, state.window.height, state.window.title);
    SetTargetFPS(state.window.target_fps);
    SetExitKey(KEY_NULL); // unbind ESC to exit

    // init raygui
    GuiLoadStyleDark();

    // init assets
    LoadAssets();

    // init game data
    state.render_texture = LoadRenderTexture(state.window.width, state.window.height);

    state.camera = (Camera2D){
        .target = (Vector2){0.0f, 0.0f},
        .offset = (Vector2){0.0f, 0.0f},
        .rotation = 0.0f,
        .zoom = 1.0f
    };

    const Vector2 ball_pos = {0, 100};
    const Vector2 ball_vel = {0, -500};
    const i32 ball_radius = 25;
    state.entities.ball = (Ball){
        .radius = ball_radius,
        .pos = ball_pos,
        .vel = ball_vel,
        .rect = (Rectangle){ball_pos.x - ball_radius, ball_pos.y - ball_radius, ball_radius * 2, ball_radius * 2},
        .anim = LoadAnimation(6, assets.ball_textures),
    };

    const Vector2 paddle_size = {200, 50};
    state.entities.paddle = (Paddle){
        .vel = (Vector2){0, 0},
        .rect = (Rectangle){
            -paddle_size.x / 2,
            (-state.window.height + paddle_size.y) / 2,
            paddle_size.x, paddle_size.y
        },
        .anim = LoadAnimation(1, assets.paddle_textures),
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

    // update entities
    Ball *ball = &state.entities.ball;
    Paddle *paddle = &state.entities.paddle;

    UpdateAnimation(&ball->anim);
    UpdateAnimation(&paddle->anim);

    const i32 screen_left   = -state.window.width / 2;
    const i32 screen_right  =  state.window.width / 2;
    const i32 screen_bottom = -state.window.height / 2;
    const i32 screen_top    =  state.window.height / 2;

    // move the paddle based on user input, and constrain it to the screen bounds
    const f32 speed = 500;
    const i32 sign = left_down ? -1 : right_down ? 1 : 0;
    paddle->vel.x = sign * speed * dt;
    paddle->rect.x += paddle->vel.x;
    paddle->rect.x = Clamp(screen_left, paddle->rect.x, screen_right - paddle->rect.width);

    // move the ball and update its bounds
    ball->pos.x += ball->vel.x * dt;
    ball->pos.y += ball->vel.y * dt;
    ball->rect.x = ball->pos.x - ball->radius;
    ball->rect.y = ball->pos.y - ball->radius;

    // clamp the ball to the screen, and bounce it if it hits the edge
    if (ball->rect.x < screen_left) {
        ball->rect.x = screen_left;
        ball->pos.x = ball->rect.x + ball->radius;
        ball->vel.x *= -1;
    } else if (ball->rect.x + ball->rect.width > screen_right) {
        ball->rect.x = screen_right - ball->rect.width;
        ball->pos.x = ball->rect.x + ball->radius;
        ball->vel.x *= -1;
    }

    if (ball->rect.y < screen_bottom) {
        ball->rect.y = screen_bottom;
        ball->pos.y = ball->rect.y + ball->radius;
        ball->vel.y *= -1;
    } else if (ball->rect.y + ball->rect.height > screen_top) {
        ball->rect.y = screen_top - ball->rect.height;
        ball->pos.y = ball->rect.y + ball->radius;
        ball->vel.y *= -1;
    }

    // test for collision with paddle and respond
    // TODO - this needs to check against each edge rather than the simple 'did they touch' test,
    //        or if there's a circle-rect test that outputs the collision normal, that could be used instead
    if (CheckCollisionCircleRec(ball->pos, ball->radius, paddle->rect)) {
        ball->vel.y *= -1;
    }

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
            texture = GetAnimationKeyframe(ball.anim);
            texture_rect = (Rectangle){0, 0, texture.width, texture.height};
            DrawTexturePro(texture, texture_rect, ball.rect, origin, 0.0f, WHITE);

            const Paddle paddle = state.entities.paddle;
            texture = GetAnimationKeyframe(paddle.anim);
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

// ----------------------------------------------------------------------------
// Asset functions

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

// ----------------------------------------------------------------------------
// Animation functions

internal Animation LoadAnimation(u32 frames_per_sec, Texture2D *textures) {
    return (Animation){
        .frames_per_sec = frames_per_sec,
        .frames_elapsed = 0,
        .current_texture = 0,
        .textures = textures
    };
}

internal Texture2D GetAnimationKeyframe(Animation anim) {
    return anim.textures[anim.current_texture];
}

internal void UpdateAnimation(Animation *anim) {
    const i32 next_texture = (state.window.target_fps / anim->frames_per_sec);

    anim->frames_elapsed++;
    if (anim->frames_elapsed >= next_texture) {
        anim->frames_elapsed -= next_texture;
        anim->current_texture = (anim->current_texture + 1) % arrlen(anim->textures);
    }
}
