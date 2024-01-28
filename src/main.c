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

global const Vector2 GRAVITY = {0, -500.0f};

global Assets assets = {0};

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

    state.entities = (struct Entities){
        .ball = MakeBall((Vector2){0, 100}, (Vector2){0, -500}, 25, LoadAnimation(6, assets.ball_textures)),
        .paddle = MakePaddle((Vector2){0, 0}, (Vector2){200, 50}, LoadAnimation(1, assets.paddle_textures)),
        .bounds = MakeArenaBounds((Rectangle) {
                -state.window.width / 2,
                -state.window.height / 2,
                state.window.width,
                state.window.height
        }),
    };

    // add mover components to world array
    arrput(state.world.movers, state.entities.ball.mover);
    arrput(state.world.movers, state.entities.paddle.mover);

    // add collider components to world array
    arrput(state.world.colliders, state.entities.ball.collider);
    arrput(state.world.colliders, state.entities.paddle.collider);
    for (i32 i = 0; i < arrlen(state.entities.bounds.colliders); i++) {
        arrput(state.world.colliders, state.entities.bounds.colliders[i]);
    }
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

    // move the paddle based on user input and limit its speed
    const f32 speed_max = 2000;
    const f32 speed_impulse = 500;
    const i32 sign = left_down ? -1 : right_down ? 1 : 0;
    paddle->mover.vel.x += sign * speed_impulse * dt;
    if (calc_abs(paddle->mover.vel.x) > speed_max) {
        paddle->mover.vel.x = calc_approach(paddle->mover.vel.x, sign * speed_max, 2000 * dt);
    }

    UpdateMover(dt, &paddle->pos, &paddle->mover, &paddle->collider);
    UpdateMover(dt, &ball->pos, &ball->mover, &ball->collider);

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
            const Rectangle ball_rect = GetRectForCircle(ball.collider.shape.circle);
            texture = GetAnimationKeyframe(ball.anim);
            texture_rect = (Rectangle){0, 0, texture.width, texture.height};
            DrawTexturePro(texture, texture_rect, ball_rect, origin, 0.0f, WHITE);

            const Paddle paddle = state.entities.paddle;
            texture = GetAnimationKeyframe(paddle.anim);
            texture_rect = (Rectangle){0, 0, texture.width, texture.height};
            DrawTexturePro(texture, texture_rect, paddle.collider.shape.rect, origin, 0.0f, WHITE);
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

// ----------------------------------------------------------------------------
// Component functions

internal bool CheckCollision(Collider *collider, CollisionMask mask, Vector2 offset) {
    if (!collider || mask == MASK_NONE) return false;

    for (i32 i = 0; i < arrlen(state.world.colliders); i++) {
        Collider *other = &state.world.colliders[i];

        bool is_different  = other != collider;
        bool is_masked     = (other->mask & mask) == mask;
        if (is_different && is_masked && CollidersOverlap(collider, other, offset)) {
            return true;
        }
    }
    return false;
}

internal bool CollidersOverlap(Collider *a, Collider *b, Vector2 offset) {
    // can't overlap if there's nothing to overlap with
    if (!a || !b) return false;

    // check for overlaps between each permutation of shape types
    if (a->type == SHAPE_RECT) {
        if (b->type == SHAPE_RECT) {
            return rect_rect_overlap(a->shape.rect, b->shape.rect, offset);
        } else if (b->type == SHAPE_CIRC) {
            return circ_rect_overlaps(b->shape.circle, a->shape.rect, offset);
        }
    } else if (a->type == SHAPE_CIRC) {
        if (b->type == SHAPE_CIRC) {
            return circ_circ_overlap(a->shape.circle, b->shape.circle, offset);
        } else if (b->type == SHAPE_RECT) {
            return circ_rect_overlaps(a->shape.circle, b->shape.rect, offset);
        }
    }

    return false;
}

internal bool MoveX(Vector2 *pos, Mover *mover, Collider *collider, i32 amount) {
    if (collider) {
        i32 sign = calc_sign(amount);

        // move by one pixel at a time until it hits something or has moved the full amount
        while (amount != 0) {
            // check for potential collision before moving this pixel
            if (CheckCollision(collider, collider->mask, (Vector2) {sign, 0})) {
                if (mover->on_hit_x) {
                    mover->on_hit_x(mover);
                } else {
                    // stop
                    mover->vel.x = 0;
                    mover->remainder.x = 0;
                }

                // moving further would hit something
                return true;
            }

            // move a pixel
            amount -= sign;
            pos->x += sign;
        }
    } else {
        // no collider, just move the full amount
        pos->x += amount;
    }

    // didn't hit anything
    return false;
}

internal bool MoveY(Vector2 *pos, Mover *mover, Collider *collider, i32 amount) {
    if (collider) {
        i32 sign = calc_sign(amount);

        // move by one pixel at a time until it hits something or has moved the full amount
        while (amount != 0) {
            // check for potential collision before moving this pixel
            if (CheckCollision(collider, collider->mask, (Vector2) {0, sign})) {
                if (mover->on_hit_y) {
                    mover->on_hit_y(mover);
                } else {
                    // stop
                    mover->vel.y = 0;
                    mover->remainder.y = 0;
                }

                // moving further would hit something
                return true;
            }

            // move a pixel
            amount -= sign;
            pos->y += sign;
        }
    } else {
        // no collider, just move the full amount
        pos->y += amount;
    }

    // didn't hit anything
    return false;
}

internal void UpdateMover(f32 dt, Vector2 *pos, Mover *mover, Collider *collider) {
    // apply friction
    if (mover->friction > 0) {
        mover->vel.x = calc_approach(mover->vel.x, 0, mover->friction * dt);
        mover->vel.y = calc_approach(mover->vel.y, 0, mover->friction * dt);
    }

    // apply gravity
    bool wont_collide_x = !collider || !CheckCollision(collider, collider->mask, (Vector2){calc_sign(mover->vel.x), 0});
    if (mover->gravity.x != 0 && wont_collide_x) {
        mover->vel.x += mover->gravity.x * dt;
    }

    bool wont_collide_y = !collider || !CheckCollision(collider, collider->mask, (Vector2){0, calc_sign(mover->vel.y)});
    if (mover->gravity.y != 0 && wont_collide_y) {
        mover->vel.y += mover->gravity.y * dt;
    }

    // get the total amount to move, including remainder from previous frame
    f32 total_move_x = mover->remainder.x + mover->vel.x * dt;
    f32 total_move_y = mover->remainder.y + mover->vel.y * dt;

    // round to nearest integer since we only move in whole pixels
    i32 move_x = calc_round(total_move_x);
    i32 move_y = calc_round(total_move_y);

    // save the remainder for next frame
    mover->remainder.x = total_move_x - move_x;
    mover->remainder.y = total_move_y - move_y;

    // move by integer values
    MoveX(pos, mover, collider, move_x);
    MoveY(pos, mover, collider, move_y);

    // update collider once movement is complete
    if (collider) {
        switch (collider->type) {
            case SHAPE_CIRC: {
                collider->shape.circle.center.x = pos->x;
                collider->shape.circle.center.y = pos->y;
                break;
            }
            case SHAPE_RECT: {
                // position is center
                collider->shape.rect.x = pos->x - collider->shape.rect.width / 2;
                collider->shape.rect.y = pos->y - collider->shape.rect.height / 2;
                break;
            }
            default: break;
        }
    }
}

// ----------------------------------------------------------------------------
// Factory functions

internal void BallHitX(Mover *self) {
    self->vel.x *= -1;
}

internal void BallHitY(Mover *self) {
    self->vel.y *= -1;
}

internal Ball MakeBall(Vector2 pos, Vector2 vel, u32 radius, Animation anim) {
    return (Ball){
            .pos = pos,
            .anim = anim,
            .mover = (Mover){
                    .vel = vel,
                    .remainder = (Vector2){0, 0},
                    .gravity = GRAVITY,
                    .friction = 0.1f,
                    .on_hit_x = BallHitX,
                    .on_hit_y = BallHitY,
            },
            .collider = (Collider){
                    .mask = MASK_BALL | MASK_PADDLE | MASK_BOUNDS,
                    .type = SHAPE_CIRC,
                    .shape = {
                            .circle = {
                                    .center = pos,
                                    .radius = radius,
                            },
                    },
            },
    };
}

internal Paddle MakePaddle(Vector2 pos, Vector2 size, Animation anim) {
    return (Paddle){
            .pos = pos,
            .anim = anim,
            .mover = (Mover){
                    .vel = {0, 0},
                    .remainder = {0, 0},
                    .gravity = {0, 0},
                    .friction = 0.75f,
                    .on_hit_x = NULL,
                    .on_hit_y = NULL,
            },
            .collider = (Collider){
                    .mask = MASK_BALL | MASK_BOUNDS,
                    .type = SHAPE_RECT,
                    .shape = {
                            .rect = {
                                    -size.x / 2,
                                    (-state.window.height + size.y) / 2,
                                    size.x, size.y
                            },
                    },
            },
    };
}

internal ArenaBounds MakeArenaBounds(Rectangle interior) {
    ArenaBounds bounds = {interior, NULL};

    const CollisionMask collides_with = MASK_BALL | MASK_PADDLE;
    Collider l = {collides_with, SHAPE_RECT};
    Collider r = {collides_with, SHAPE_RECT};
    Collider t = {collides_with, SHAPE_RECT};
    Collider b = {collides_with, SHAPE_RECT};

    const i32 size = 10;
    l.shape.rect = (Rectangle){interior.x - size, interior.y, size, interior.height};
    r.shape.rect = (Rectangle){interior.x + interior.width, interior.y, size, interior.height};
    t.shape.rect = (Rectangle){interior.x, interior.y + interior.height, interior.width, size};
    b.shape.rect = (Rectangle){interior.x, interior.y - size, interior.width, size};

    arrput(bounds.colliders, l);
    arrput(bounds.colliders, r);
    arrput(bounds.colliders, b);
    arrput(bounds.colliders, t);

    return bounds;
}
