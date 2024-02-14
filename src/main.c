#include "game.h"

#include "raygui.h"
#include "dark/style_dark.h"

// ----------------------------------------------------------------------------
// Global data

internal const Vector2 GRAVITY = {0, -50.0f};

Assets assets = {0};
World world = {0};
State state = {
    .window = {
        .target_fps = 60,
        .width = 1280,
        .height = 720,
        .title = "Prong"
    },
    .debug = {
        .log = false,
        .draw_colliders = true,
        .manual_frame_step = false,
    },
    .current_screen = TITLE,
};

internal void BallHitX2(Entity entity, Entity collided_with) {
    world.movements.vel_x[entity] *= -1;
    world.movements.remainder_x[entity] = 0;
}

internal void BallHitY2(Entity entity, Entity collided_with) {
    world.movements.vel_y[entity] *= -1;
    world.movements.remainder_y[entity] = 0;
}

// ----------------------------------------------------------------------------
// Entry point

int main() {
    Init();
    while (!state.input_frame.exit_requested) {
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
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(state.window.width, state.window.height, state.window.title);
    SetTargetFPS(state.window.target_fps);
    SetExitKey(KEY_NULL); // unbind ESC to exit

    // init raygui
    GuiLoadStyleDark();

    // init assets
    LoadAssets();

    // init game data
    state.render_texture = LoadRenderTexture(state.window.width, state.window.height);

    Vector2 window_center = {state.window.width / 2, state.window.height / 2};
    state.camera = (Camera2D){
        .target = (Vector2){0.0f, 0.0f},
        .offset = window_center,
        .rotation = 0.0f,
        .zoom = 1.0f
    };

    world_init();

    f32 ball_radius = 25;
    Vector2 ball_pos = {0, 100};
    Vector2 ball_vel = {-100, -200};
    Vector2 paddle_size = {200, 50};
    Vector2 paddle_center = {0, (-state.window.height + paddle_size.y) / 2};

    state.ball = world_create_entity();
    entity_add_name(state.ball, (NameStr) {"ball"});
    entity_add_position(state.ball, ball_pos.x, ball_pos.y);
    entity_add_velocity(state.ball, ball_vel.x, ball_vel.y, 0, GRAVITY.y);
    entity_add_collider_circ(state.ball, MASK_BALL, 0, 0, ball_radius);
    world.colliders.on_hit_x[state.ball] = BallHitX2;
    world.colliders.on_hit_y[state.ball] = BallHitY2;

    state.paddle = world_create_entity();
    entity_add_name(state.paddle, (NameStr) {"paddle"});
    entity_add_position(state.paddle, paddle_center.x, paddle_center.y);
    entity_add_velocity(state.paddle, 0, 0, 0.75f, 0);
    entity_add_collider_rect(state.paddle, MASK_PADDLE, paddle_size.x / 2, paddle_size.y / 2, paddle_size.x, paddle_size.y);

    // setup arena bounds
    state.bounds_l = world_create_entity(); entity_add_name(state.bounds_l, (NameStr) {"bounds_l"});
    state.bounds_r = world_create_entity(); entity_add_name(state.bounds_r, (NameStr) {"bounds_r"});
    state.bounds_t = world_create_entity(); entity_add_name(state.bounds_t, (NameStr) {"bounds_t"});
    state.bounds_b = world_create_entity(); entity_add_name(state.bounds_b, (NameStr) {"bounds_b"});

    i32 size = 10;
    Rectangle interior = (Rectangle) {-window_center.x, -window_center.y, state.window.width, state.window.height};
    entity_add_position(state.bounds_l, interior.x                  - size / 2, interior.y + interior.height / 2);
    entity_add_position(state.bounds_r, interior.x + interior.width + size / 2, interior.y + interior.height / 2);
    entity_add_position(state.bounds_t, interior.x + interior.width / 2,        interior.y + interior.height + size / 2);
    entity_add_position(state.bounds_b, interior.x + interior.width / 2,        interior.y                   - size / 2);

    entity_add_collider_rect(state.bounds_l, MASK_BOUNDS, -size / 2, -interior.height / 2, size, interior.height);
    entity_add_collider_rect(state.bounds_r, MASK_BOUNDS, -size / 2, -interior.height / 2, size, interior.height);
    entity_add_collider_rect(state.bounds_t, MASK_BOUNDS, -interior.width / 2, -size / 2, interior.width, size);
    entity_add_collider_rect(state.bounds_b, MASK_BOUNDS, -interior.width / 2, -size / 2, interior.width, size);
}

internal void Update() {
    switch (state.current_screen) {
        case TITLE: {
//            if (IsKeyReleased(KEY_ENTER) ||
//                IsKeyReleased(KEY_SPACE) ||
//                IsGestureDetected(GESTURE_TAP)) {
                state.current_screen = GAMEPLAY;
//            }
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

    // collect input
    state.input_frame = (struct InputFrame){
            .exit_requested = WindowShouldClose() || IsKeyPressed(KEY_ESCAPE),
            .move_left  = IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A),
            .move_right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D),
            .step_frame = IsKeyPressed(KEY_SPACE),
    };

    // toggle debug flags if needed
    if (IsKeyPressed(KEY_ONE))   state.debug.manual_frame_step = !state.debug.manual_frame_step;
    if (IsKeyPressed(KEY_TWO))   state.debug.draw_colliders    = !state.debug.draw_colliders;
    if (IsKeyPressed(KEY_THREE)) state.debug.log               = !state.debug.log;

    // if manual frame stepping is enabled, only update if the user has requested it
    if (state.debug.manual_frame_step && !state.input_frame.step_frame) {
        return;
    }

    // update camera
    state.camera.target = (Vector2){0, 0};
    state.camera.offset = (Vector2){state.window.width / 2, state.window.height / 2};
    state.camera.rotation = 0.0f;
    state.camera.zoom = 1.0f;

    // process paddle movement input
    if (state.input_frame.move_left || state.input_frame.move_right) {
        const f32 speed_max = 2000;
        const f32 speed_impulse = 500;
        const i32 sign = state.input_frame.move_left ? -1 : state.input_frame.move_right ? 1 : 0;

        // if the paddle is moving in the opposite direction, stop it
        bool switch_direction = sign != calc_sign(world.movements.vel_x[state.paddle]);
        if (switch_direction) {
            world.movements.vel_x[state.paddle] = 0;
        }

        // move the paddle based on user input, with an extra boost if we just switched direction
        const f32 speed_boost = switch_direction ? 50 : 1;
        world.movements.vel_x[state.paddle] += sign * speed_boost * speed_impulse * dt;

        // constrain the paddle's max speed
        if (calc_abs(world.movements.vel_x[state.paddle]) > speed_max) {
            world.movements.vel_x[state.paddle] = calc_approach(world.movements.vel_x[state.paddle], sign * speed_max, 2000 * dt);
        }
    } else {
        // always be slowing when no input
        world.movements.vel_x[state.paddle] = calc_approach(world.movements.vel_x[state.paddle], 0, 2000 * dt);
        world.movements.vel_y[state.paddle] = calc_approach(world.movements.vel_y[state.paddle], 0, 2000 * dt);
    }

    // update entities
    world_update(dt);
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

            i32 pos_x, pos_y, off_x, off_y, width, height, radius;

            pos_x = world.positions.x[state.ball];
            pos_y = world.positions.y[state.ball];
            off_x = world.colliders.offset_x[state.ball];
            off_y = world.colliders.offset_y[state.ball];
            radius = world.colliders.radius[state.ball];
            DrawCircleGradient(pos_x + off_x, pos_y + off_y, radius, BLUE, YELLOW);

            pos_x = world.positions.x[state.paddle];
            pos_y = world.positions.y[state.paddle];
            off_x = world.colliders.offset_x[state.paddle];
            off_y = world.colliders.offset_y[state.paddle];
            width = world.colliders.width[state.paddle];
            height = world.colliders.height[state.paddle];
            DrawRectangleGradientV(pos_x + off_x, pos_y + off_y, width, height, RED, GREEN);


            if (state.debug.draw_colliders) {
                Color debug_color = MAGENTA;
                for (u32 i = 0; i < world.num_entities; i++) {
                    if (i == ENTITY_NONE) continue;

                    bool has_position = entity_has_components(i, COMPONENT_POSITION);
                    bool has_collider = entity_has_components(i, COMPONENT_COLLIDER);
                    if (!has_position || !has_collider) continue;

                    pos_x = world.positions.x[i];
                    pos_y = world.positions.y[i];
                    off_x = world.colliders.offset_x[i];
                    off_y = world.colliders.offset_y[i];

                    switch (world.colliders.shape[i]) {
                        case SHAPE_CIRC: {
                            radius = world.colliders.radius[i];
                            DrawCircleLines(pos_x + off_x, pos_y + off_y, radius, debug_color);
                        } break;
                        case SHAPE_RECT: {
                            width = world.colliders.width[i];
                            height = world.colliders.height[i];
                            DrawRectangleLines(pos_x + off_x, pos_y + off_y, width, height, debug_color);
                        } break;

                        case SHAPE_NONE:
                        default: break;
                    }
                }
            }
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
    if (state.debug.manual_frame_step) {
        DrawText("frame step enabled", 10, 10, 20, state.input_frame.step_frame ? GREEN : WHITE);
    }
    EndDrawing();
}

internal void Shutdown() {
    world_cleanup();
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

// ----------------------------------------------------------------------------
// Include single file header implementations

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
