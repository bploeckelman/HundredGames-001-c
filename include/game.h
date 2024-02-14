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

global inline f32 calc_clamp(f32 val, f32 min, f32 max) {
    return (val < min) ? min : ((val > max) ? max : val);
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
// Entity Component System

// an entity is just an index into arrays of components
typedef u32 Entity;

// the zero-th entity is reserved for "no entity"
global const Entity ENTITY_NONE = 0;

// wrap a fixed length string in a struct to simplify usage
#define NAME_MAX_LEN 256
typedef struct {
    char val[NAME_MAX_LEN];
} NameStr;
global const NameStr NAME_EMPTY = {0};

typedef struct {
    NameStr *name;
} Names;

typedef struct {
    i32 *x;
    i32 *y;
    i32 *prev_x;
    i32 *prev_y;
} Positions;

typedef struct {
    f32 *vel_x;
    f32 *vel_y;
    f32 *remainder_x;
    f32 *remainder_y;
    f32 *friction;
    f32 *gravity;
} Movements;

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
} Shape;

typedef void (*OnHitFunc)(Entity entity, Entity collided_with);
typedef struct {
    // offsets from entity position, typically {0, 0}
    i32 *offset_x;
    i32 *offset_y;
    u32 *width;
    u32 *height;
    u32 *radius;
    Shape *shape;
    CollisionMask *mask;
    OnHitFunc *on_hit_x;
    OnHitFunc *on_hit_y;
} Colliders;

typedef u32 ComponentMask;
enum {
    COMPONENT_NONE     = 0,
    COMPONENT_NAME     = (1 << 0),
    COMPONENT_POSITION = (1 << 1),
    COMPONENT_MOVEMENT = (1 << 2),
    COMPONENT_COLLIDER = (1 << 3),
};

typedef struct {
    bool *in_use;
    bool *active;
    ComponentMask *components;
} EntityInfos;

typedef struct {
    bool initialized;

    Entity num_entities;
    EntityInfos infos;

    Names names;
    Positions positions;
    Movements movements;
    Colliders colliders;
} World;

extern World world;


void world_init();
void world_update(f32 dt);
void world_cleanup();

Entity world_create_entity();
void world_destroy_entity(Entity entity);

bool entity_has_components(Entity entity, ComponentMask mask);

void entity_add_name(Entity entity, NameStr name);
void entity_add_position(Entity entity, u32 x, u32 y);
void entity_add_velocity(Entity entity, f32 vel_x, f32 vel_y, f32 friction, f32 gravity);
void entity_add_collider_rect(Entity entity, CollisionMask mask, u32 offset_x, u32 offset_y, u32 width, u32 height);
void entity_add_collider_circ(Entity entity, CollisionMask mask, u32 offset_x, u32 offset_y, u32 radius);

global inline bool rect_rect_overlaps(i32 x1, i32 y1, i32 w1, i32 h1, i32 x2, i32 y2, i32 w2, i32 h2) {
    Rectangle a = { x1, y1, w1, h1 };
    Rectangle b = { x2, y2, w2, h2 };
    return CheckCollisionRecs(a, b);
}

global inline bool circ_rect_overlaps(i32 cx, i32 cy, i32 cr, i32 rx, i32 ry, i32 rw, i32 rh) {
    Vector2 c = { cx, cy };
    Rectangle r = { rx, ry, rw, rh };
    return CheckCollisionCircleRec(c, cr, r);
}

global inline bool circ_circ_overlaps(i32 x1, i32 y1, i32 r1, i32 x2, i32 y2, i32 r2) {
    Vector2 c1 = { x1, y1 };
    Vector2 c2 = { x2, y2 };
    return CheckCollisionCircles(c1, r1, c2, r2);
}

void circ_circ_resolve(Entity entity, Entity collided_with);
void circ_rect_resolve(Entity entity, Entity collided_with);
void rect_rect_resolve(Entity entity, Entity collided_with);

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
        bool log;
        bool draw_colliders;
        bool manual_frame_step;
    } debug;

    struct InputFrame {
        bool exit_requested;
        bool move_left;
        bool move_right;
        bool step_frame;
    } input_frame;

    Entity ball;
    Entity paddle;
    Entity bounds_l;
    Entity bounds_r;
    Entity bounds_t;
    Entity bounds_b;

    GameScreen current_screen;
    RenderTexture render_texture;
    Camera2D camera;
} State;

extern State state;

// ----------------------------------------------------------------------------
// Assets

typedef struct {
    Texture2D *ball_textures ;
    Texture2D *paddle_textures;
} Assets;

extern Assets assets;

void LoadAssets();
void UnloadAssets();
