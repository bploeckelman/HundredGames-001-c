#pragma once

#include "common.h"
#include "raylib.h"

// ----------------------------------------------------------------------------
// Convenience functions

global inline i32 calc_sign(f32 val) {
    return (val > 0) ? 1 : ((val < 0) ? -1 : 0);
}

global inline f32 calc_min(f32 a, f32 b) {
    return (a < b) ? a : b;
}

global inline f32 calc_max(f32 a, f32 b) {
    return (a > b) ? a : b;
}

global inline f32 calc_abs(f32 val) {
    return (val < 0) ? -val : val;
}

global inline i32 calc_round(f32 val) {
    return (i32)roundf(val);
}

global inline f32 calc_approach(f32 t, f32 target, f32 delta) {
    return (t < target) ? calc_min(t + delta, target) : calc_max(t - delta, target);
}

global inline f32 calc_unit_random() {
    return GetRandomValue(0, 1000) / 1000.0f;
}

global inline f32 calc_signed_random() {
    return (GetRandomValue(0, 1000) / 500.0f) - 1.0f;
}

// ----------------------------------------------------------------------------
// Game lifecycle functions

void Init();
void Update();
void UpdateGameplay();
void DrawFrame();
void Shutdown();

// ----------------------------------------------------------------------------
// Game screens

typedef enum {
    TITLE = 0,
    GAMEPLAY,
    CREDITS,
} GameScreen;

// ----------------------------------------------------------------------------
// Helpers

typedef struct {
    u32 frames_per_sec;
    u32 frames_elapsed;
    i32 current_texture;
    Texture2D *textures;
} Animation;

Animation LoadAnimation(u32 frames_per_sec, Texture2D *textures);
Texture2D GetAnimationKeyframe(Animation anim);
void UpdateAnimation(Animation *anim);

// ----------------------------------------------------------------------------
// Components

typedef u32 EntityID;
global const EntityID ENTITY_ID_NONE = 0;
global EntityID next_entity_id = 1;

typedef struct {
    Vector2 center;
    f32 radius;
} Circle;

typedef u32 CollisionMask;
enum {
    MASK_NONE   = 0,
    MASK_BALL   = (1 << 0),
    MASK_PADDLE = (1 << 1),
    MASK_BOUNDS = (1 << 2),
};

typedef enum {
    SHAPE_NONE = 0,
    SHAPE_CIRC,
    SHAPE_RECT,
    SHAPE_COUNT,
} ShapeType;

typedef struct {
    EntityID entity_id;
    u32 mask;
    u32 collides_with;
    ShapeType type;
    union {
        Circle circle;
        Rectangle rect;
    } shape;
} Collider;

typedef struct Mover Mover;
struct Mover {
    EntityID entity_id;
    Vector2 vel;
    Vector2 remainder;
    Vector2 gravity;
    f32 friction;
    void (*on_hit_x)(Mover *self, EntityID collided_with_id);
    void (*on_hit_y)(Mover *self, EntityID collided_with_id);
};

EntityID CheckForCollisions(Collider *collider, Vector2 offset);
bool CollidersOverlap(Collider *a, Collider *b, Vector2 offset);
void UpdateMover(f32 dt, Vector2 *pos, Mover *mover, Collider *collider);

global inline bool rect_rect_overlaps(Rectangle a, Rectangle b, Vector2 offset) {
    Rectangle a_offset = { a.x + offset.x, a.y + offset.y, a.width, a.height };
    return CheckCollisionRecs(a_offset, b);
}

global inline bool circ_rect_overlaps(Circle c, Rectangle r, Vector2 offset) {
    Vector2 c_offset = { c.center.x + offset.x, c.center.y + offset.y };
    return CheckCollisionCircleRec(c_offset, c.radius, r);
}

global inline bool circ_circ_overlaps(Circle a, Circle b, Vector2 offset) {
    Vector2 a_offset = { a.center.x + offset.x, a.center.y + offset.y };
    return CheckCollisionCircles(a_offset, a.radius, b.center, b.radius);
}

global inline Rectangle GetRectForCircle(Circle c) {
    return (Rectangle){ c.center.x - c.radius, c.center.y - c.radius, c.radius * 2, c.radius * 2 };
}

// ----------------------------------------------------------------------------
// Game objects

typedef struct {
    EntityID entity_id;
    Vector2 pos;
    Animation anim;
    Mover mover;
    Collider collider;
} Ball;

typedef struct {
    EntityID entity_id;
    Vector2 pos;
    Animation anim;
    Mover mover;
    Collider collider;
} Paddle;

typedef struct {
    EntityID entity_id;
    Rectangle interior;
    Collider *colliders;
} ArenaBounds;

// ----------------------------------------------------------------------------
// Game state data

typedef struct {
    struct Window {
        i32 target_fps;
        i32 width;
        i32 height;
        char* title;
    } window;

    struct Debug {
        bool draw_colliders;
        bool manual_frame_step;
    } debug;

    struct InputFrame {
        bool exit_requested;
        bool move_left;
        bool move_right;
        bool step_frame;
    } input_frame;

    struct Entities {
        Ball ball;
        Paddle paddle;
        ArenaBounds bounds;
    } entities;

    struct World {
        Mover **movers;
        Collider **colliders;
    } world;

    GameScreen current_screen;
    RenderTexture render_texture;
    Camera2D camera;
} State;

// ----------------------------------------------------------------------------
// Assets

typedef struct {
    Texture2D *ball_textures ;
    Texture2D *paddle_textures;
} Assets;

void LoadAssets();
void UnloadAssets();

// ----------------------------------------------------------------------------
// Factory functions

Ball MakeBall(Vector2 center_pos, Vector2 vel, u32 radius, Animation anim);
Paddle MakePaddle(Vector2 center_pos, Vector2 size, Animation anim);
ArenaBounds MakeArenaBounds(Rectangle interior);
