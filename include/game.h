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
// Entity Component System

// an entity is just an index into arrays of components
typedef u32 Entity;
global const Entity ENTITY_NONE = 0;
global Entity next_entity = 1; // old...


// wrap a fixed length string in a struct to simplify usage
#define NAME_MAX_LEN 256
typedef struct {
    char val[NAME_MAX_LEN];
} NameStr;
global const NameStr NAME_EMPTY = {0};

typedef struct {
    // flag indicating whether the entity for each element has this component or not
    bool *active;

    NameStr *name;
} Name;

typedef struct {
    // flag indicating whether the entity for each element has this component or not
    bool *active;

    u32 *x;
    u32 *y;
    u32 *prev_x;
    u32 *prev_y;
} Position;

typedef struct {
    // flag indicating whether the entity for each element has this component or not
    bool *active;

    f32 *vel_x;
    f32 *vel_y;
    f32 *remainder_x;
    f32 *remainder_y;
} Velocity;

// TODO - how to store different shape types and their data here...
//  maybe include all elements and use the type field to set the primary shape,
//  that way if the primary shape is a circle, the rect fields represent a bounding rect for the circle
//  similarly if the primary shape is a rectangle, the circle field represents a bounding circle
//  for a polygon, the circle and rect fields would also be bounding shapes, used for broad phase collisions
typedef struct {
    // flag indicating whether the entity for each element has this component or not
    bool *active;

    // offsets from entity position, typically {0, 0}
    u32 *offset_x;
    u32 *offset_y;
    u32 *width;
    u32 *height;
    u32 *radius;
} ColliderShape;



// TODO - technically these are sparse arrays, since not all entities will have all components
// TODO - instead of having a separate 'active' array for each component type,
//  a 'world' level bitfield could be used to indicate which entities have which components
typedef struct {
    Entity num_entities;

    Name names;
    Position positions;
    Velocity velocities;
    ColliderShape collider_shapes;
} World;


extern World world;

void ecs_init();
void ecs_update();
void ecs_cleanup();

Entity ecs_create_entity();
void ecs_destroy_entity(Entity entity);

void ecs_add_name(Entity entity, NameStr name);
void ecs_add_position(Entity entity, u32 x, u32 y);
void ecs_add_velocity(Entity entity, f32 vel_x, f32 vel_y);
void ecs_add_collider(Entity entity, u32 offset_x, u32 offset_y, u32 width, u32 height, u32 radius);



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
    Entity entity_id;
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
    Entity entity_id;
    Vector2 vel;
    Vector2 remainder;
    Vector2 gravity;
    f32 friction;
    void (*on_hit_x)(Mover *self, Entity collided_with_id);
    void (*on_hit_y)(Mover *self, Entity collided_with_id);
};

Entity CheckForCollisions(Collider *collider, u32 mask, Vector2 offset);
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
    Entity entity_id;
    Vector2 pos;
    Animation anim;
    Mover mover;
    Collider collider;
} Ball;

typedef struct {
    Entity entity_id;
    Vector2 pos;
    Animation anim;
    Mover mover;
    Collider collider;
} Paddle;

typedef struct {
    Entity entity_id;
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
