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
        .draw_colliders = true,
        .manual_frame_step = false,
    },
    .current_screen = TITLE,
};

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

    f32 ball_radius = 25;
    Vector2 ball_pos = {0, 100};
    Vector2 ball_vel = {0, -200};
    Vector2 paddle_size = {200, 50};
    Vector2 paddle_center = {0, (-state.window.height + paddle_size.y) / 2};
//    state.entities = (struct Entities){
//        .ball = MakeBall(ball_pos, ball_vel, ball_radius, LoadAnimation(6, assets.ball_textures)),
//        .paddle = MakePaddle(paddle_center, paddle_size, LoadAnimation(1, assets.paddle_textures)),
//        .bounds = MakeArenaBounds((Rectangle) {
//                -window_center.x,
//                -window_center.y,
//                state.window.width,
//                state.window.height
//        }),
//    };

    world_init();

    state.ball = world_create_entity();
    entity_add_name(state.ball, (NameStr) {"ball"});
    entity_add_position(state.ball, ball_pos.x, ball_pos.y);
    entity_add_velocity(state.ball, ball_vel.x, ball_vel.y, 0, GRAVITY.y);
    entity_add_collider(state.ball, -ball_radius, -ball_radius, 2*ball_radius, 2*ball_radius, ball_radius);

    state.paddle = world_create_entity();
    entity_add_name(state.paddle, (NameStr) {"paddle"});
    entity_add_position(state.paddle, paddle_center.x, paddle_center.y);
    entity_add_velocity(state.paddle, 0, 0, 0.75f, 0);
    entity_add_collider(state.paddle, paddle_size.x / 2, paddle_size.y / 2, paddle_size.x, paddle_size.y, calc_max(paddle_size.x, paddle_size.y) / 2);

    // add mover components to world array
//    arrput(state.world.movers, &state.entities.ball.mover);
//    arrput(state.world.movers, &state.entities.paddle.mover);

    // add collider components to world array
//    arrput(state.world.colliders, &state.entities.ball.collider);
//    arrput(state.world.colliders, &state.entities.paddle.collider);
//    for (i32 i = 0; i < arrlen(state.entities.bounds.colliders); i++) {
//        arrput(state.world.colliders, &state.entities.bounds.colliders[i]);
//    }
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
    if (IsKeyPressed(KEY_ONE)) state.debug.manual_frame_step = !state.debug.manual_frame_step;
    if (IsKeyPressed(KEY_TWO)) state.debug.draw_colliders    = !state.debug.draw_colliders;

    // if manual frame stepping is enabled, only update if the user has requested it
    if (state.debug.manual_frame_step && !state.input_frame.step_frame) {
        return;
    }

    // update entities
//    Ball *ball = &state.entities.ball;
//    Paddle *paddle = &state.entities.paddle;
//
//    UpdateAnimation(&ball->anim);
//    UpdateAnimation(&paddle->anim);
//
//    // move the paddle based on user input and limit its speed
//    if (state.input_frame.move_left || state.input_frame.move_right) {
//        const f32 speed_max = 2000;
//        const f32 speed_impulse = 500;
//        const i32 sign = state.input_frame.move_left ? -1 : state.input_frame.move_right ? 1 : 0;
//
//        // if the paddle is moving in the opposite direction, stop it
//        bool switch_direction = sign != calc_sign(paddle->mover.vel.x);
//        if (switch_direction) {
//            paddle->mover.vel.x = 0;
//        }
//
//        // move the paddle based on user input, with an extra boost if we just switched direction
//        const f32 speed_boost = switch_direction ? 50 : 1;
//        paddle->mover.vel.x += sign * speed_boost * speed_impulse * dt;
//
//        // constrain the paddle's max speed
//        if (calc_abs(paddle->mover.vel.x) > speed_max) {
//            paddle->mover.vel.x = calc_approach(paddle->mover.vel.x, sign * speed_max, 2000 * dt);
//        }
//    } else {
//        paddle->mover.vel.x = calc_approach(paddle->mover.vel.x, 0, 2000 * dt);
//        paddle->mover.vel.y = calc_approach(paddle->mover.vel.y, 0, 2000 * dt);
//    }
//
//    UpdateMover(dt, &paddle->pos, &paddle->mover, &paddle->collider);
//    UpdateMover(dt, &ball->pos, &ball->mover, &ball->collider);

    // update camera
    state.camera.target = (Vector2){0, 0};
    state.camera.offset = (Vector2){state.window.width / 2, state.window.height / 2};
    state.camera.rotation = 0.0f;
    state.camera.zoom = 1.0f;

    // TODO - process input to update velocity for paddle
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

//            const Ball ball = state.entities.ball;
//            const Rectangle ball_rect = GetRectForCircle(ball.collider.shape.circle);
//            texture = GetAnimationKeyframe(ball.anim);
//            texture_rect = (Rectangle){0, 0, texture.width, texture.height};
//            DrawTexturePro(texture, texture_rect, ball_rect, origin, 0.0f, WHITE);

//            const Paddle paddle = state.entities.paddle;
//            texture = GetAnimationKeyframe(paddle.anim);
//            texture_rect = (Rectangle){0, 0, texture.width, texture.height};
//            DrawTexturePro(texture, texture_rect, paddle.collider.shape.rect, origin, 0.0f, WHITE);

            i32 pos_x = world.positions.x[state.ball];
            i32 pos_y = world.positions.y[state.ball];
            i32 off_x = world.collider_shapes.offset_x[state.ball];
            i32 off_y = world.collider_shapes.offset_y[state.ball];
            i32 width = world.collider_shapes.width[state.ball];
            i32 height = world.collider_shapes.height[state.ball];
            DrawRectangleGradientH(pos_x + off_x, pos_y + off_y, width, height, BLUE, YELLOW);

            pos_x = world.positions.x[state.paddle];
            pos_y = world.positions.y[state.paddle];
            off_x = world.collider_shapes.offset_x[state.paddle];
            off_y = world.collider_shapes.offset_y[state.paddle];
            width = world.collider_shapes.width[state.paddle];
            height = world.collider_shapes.height[state.paddle];
            DrawRectangleGradientV(pos_x + off_x, pos_y + off_y, width, height, RED, GREEN);


            if (state.debug.draw_colliders) {
//                for (u32 i = 0; i < arrlen(state.world.colliders); i++) {
//                    Collider *collider = state.world.colliders[i];
//                    switch (collider->type) {
//                        case SHAPE_CIRC: {
//                            DrawCircleLinesV(collider->shape.circle.center, collider->shape.circle.radius, MAGENTA);
//                            break;
//                        }
//                        case SHAPE_RECT: {
//                            DrawRectangleLinesEx(collider->shape.rect, 1, MAGENTA);
//                            break;
//                        }
//                        case SHAPE_NONE:
//                        default: break;
//                    }
//                }
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
// Component functions

internal Entity CheckForCollisions(Collider *collider, u32 mask, Vector2 offset) {
    if (!collider || collider->collides_with == MASK_NONE) return false;

//    for (i32 i = 0; i < arrlen(state.world.colliders); i++) {
//        Collider *other = state.world.colliders[i];
//
//        bool is_different  = collider != other;
//        bool collides_with = (collider->mask & mask) == mask;
//        if (is_different && collides_with && CollidersOverlap(collider, other, offset)) {
//            return other->entity_id;
//        }
//    }
    return ENTITY_NONE;
}

internal bool CollidersOverlap(Collider *a, Collider *b, Vector2 offset) {
    // can't overlap if there's nothing to overlap with
    if (!a || !b) return false;

    // check for overlaps between each permutation of shape types
    switch (a->type) {
        case SHAPE_CIRC: {
            switch (b->type) {
                case SHAPE_CIRC: return circ_circ_overlaps(a->shape.circle, b->shape.circle, offset);
                case SHAPE_RECT: return circ_rect_overlaps(a->shape.circle, b->shape.rect, offset);
                case SHAPE_NONE:
                default: break;
            } break;
        }
        case SHAPE_RECT: {
            switch (b->type) {
                case SHAPE_RECT: return rect_rect_overlaps(a->shape.rect, b->shape.rect, offset);
                case SHAPE_CIRC: return circ_rect_overlaps(b->shape.circle, a->shape.rect, offset);
                case SHAPE_NONE:
                default: break;
            } break;
        }
        case SHAPE_NONE:
        default: break;
    }

    return false;
}

internal void UpdateColliderPos(Vector2 *pos, Collider *collider) {
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
            case SHAPE_NONE:
            default: break;
        }
    }
}

internal bool MoveX(Vector2 *pos, Mover *mover, Collider *collider, i32 amount) {
    if (collider) {
        i32 sign = calc_sign(amount);

        // move by one pixel at a time until it hits something or has moved the full amount
        while (amount != 0) {
            // check for potential collision with the world before moving this pixel
            Entity collided_with = CheckForCollisions(collider, MASK_BOUNDS, (Vector2) {sign, 0});
            if (collided_with != ENTITY_NONE) {
                if (mover->on_hit_x) {
                    mover->on_hit_x(mover, collided_with);
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
            UpdateColliderPos(pos, collider);
        }
    } else {
        // no collider, just move the full amount
        pos->x += amount;
        UpdateColliderPos(pos, collider);
    }

    // didn't hit anything
    return false;
}

internal bool MoveY(Vector2 *pos, Mover *mover, Collider *collider, i32 amount) {
    if (collider) {
        i32 sign = calc_sign(amount);

        // move by one pixel at a time until it hits something or has moved the full amount
        while (amount != 0) {
            // check for potential collision with the world before moving this pixel
            Entity collided_with = CheckForCollisions(collider, MASK_BOUNDS, (Vector2) {sign, 0});
            if (collided_with != ENTITY_NONE) {
                if (mover->on_hit_y) {
                    mover->on_hit_y(mover, collided_with);
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
            UpdateColliderPos(pos, collider);
        }
    } else {
        // no collider, just move the full amount
        pos->y += amount;
        UpdateColliderPos(pos, collider);
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
    if (mover->gravity.x != 0) mover->vel.x += mover->gravity.x * dt;
    if (mover->gravity.y != 0) mover->vel.y += mover->gravity.y * dt;

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
}

// ----------------------------------------------------------------------------
// Factory functions

internal void BallHitX(Mover *self, Entity collided_with_id) {
    self->vel.x *= -1;
    self->remainder.x = 0;

    // if the thing we hit was a paddle, impart some of its velocity to the ball
//    if (collided_with_id == state.entities.paddle.entity_id) {
//        printf("ball hit paddle - x\n");
//
//        const f32 speed_transmit_scale = 0.5f;
//        if (state.entities.paddle.mover.vel.x != 0) {
//            self->vel.x += state.entities.paddle.mover.vel.x * speed_transmit_scale;
//        }
//    }
}

internal void BallHitY(Mover *self, Entity collided_with_id) {
    self->vel.y *= -1;
    self->remainder.y = 0;

    // if the thing we hit was a paddle, impart some of its velocity to the ball
//    if (collided_with_id == state.entities.paddle.entity_id) {
//        printf("ball hit paddle - y\n");
//
//        const f32 speed_transmit_scale = 0.5f;
//        if (state.entities.paddle.mover.vel.x != 0) {
//            self->vel.x += state.entities.paddle.mover.vel.x * speed_transmit_scale;
//        }
//    }
}

internal Ball MakeBall(Vector2 center_pos, Vector2 vel, u32 radius, Animation anim) {
    Entity entity_id = next_entity++;
    return (Ball){
            .entity_id = entity_id,
            .pos = center_pos,
            .anim = anim,
            .mover = (Mover){
                    .entity_id = entity_id,
                    .vel = vel,
                    .remainder = (Vector2){0, 0},
                    .gravity = GRAVITY,
                    .friction = 0,
                    .on_hit_x = BallHitX,
                    .on_hit_y = BallHitY,
            },
            .collider = (Collider){
                    .entity_id = entity_id,
                    .mask = MASK_BALL,
                    .collides_with = MASK_BALL | MASK_PADDLE | MASK_BOUNDS,
                    .type = SHAPE_CIRC,
                    .shape = {
                            .circle = {
                                    .center = center_pos,
                                    .radius = radius,
                            },
                    },
            },
    };
}

internal Paddle MakePaddle(Vector2 center_pos, Vector2 size, Animation anim) {
    Entity entity_id = next_entity++;
    return (Paddle){
            .entity_id = entity_id,
            .pos = center_pos,
            .anim = anim,
            .mover = (Mover){
                    .entity_id = entity_id,
                    .vel = {0, 0},
                    .remainder = {0, 0},
                    .gravity = {0, 0},
                    .friction = 0.9f,
                    .on_hit_x = NULL,
                    .on_hit_y = NULL,
            },
            .collider = (Collider){
                    .entity_id = entity_id,
                    .mask = MASK_PADDLE,
                    .collides_with = MASK_BALL | MASK_BOUNDS,
                    .type = SHAPE_RECT,
                    .shape = {
                            .rect = {
                                    center_pos.x - size.x / 2,
                                    center_pos.y - size.y / 2,
                                    size.x, size.y
                            },
                    },
            },
    };
}

internal ArenaBounds MakeArenaBounds(Rectangle interior) {
    Entity entity_id = next_entity++;
    ArenaBounds bounds = {entity_id, interior, NULL};

    u32 mask = MASK_BOUNDS;
    u32 collides_with = MASK_BALL | MASK_PADDLE;
    Collider l = {entity_id, mask, collides_with, SHAPE_RECT};
    Collider r = {entity_id, mask, collides_with, SHAPE_RECT};
    Collider t = {entity_id, mask, collides_with, SHAPE_RECT};
    Collider b = {entity_id, mask, collides_with, SHAPE_RECT};

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

// ----------------------------------------------------------------------------
// Include single file header implementations

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
