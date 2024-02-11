#include "game.h"

internal void entity_create_names();
internal void entity_create_positions();
internal void entity_create_velocities();
internal void entity_create_colliders();

internal void entity_cleanup_names();
internal void entity_cleanup_positions();
internal void entity_cleanup_velocities();
internal void entity_cleanup_colliders();

internal void world_log();

void world_init() {
    if (world.active) {
        world_cleanup();
    }

    world = (World) {0};
    world.active = true;

    // reserve the '0' entity id to represent 'no entity'
    world_create_entity();
    entity_add_name(0, (NameStr) {"ENTITY_NONE"});
}

void world_update() {
    // just print for now...
    world_log();
}

void world_cleanup() {
    entity_cleanup_names();
    entity_cleanup_positions();
    entity_cleanup_velocities();
    entity_cleanup_colliders();
    world = (World) {0};
}

Entity world_create_entity() {
    // ensure that we have an initialized world before creating any entities
    if (!world.active) {
        world_init();
    }

    // return the next unused entity id
    u32 next_entity_id = world.num_entities++;

    // TODO - scan for free slots first, if there is one, pass the index into the `ecs_create_entity_<component>()` functions
    //  to zero out the arrays for that entity's slot, see comment in world_destroy_entity()

    // add an 'empty' element to each component array for the new entity
    entity_create_names();
    entity_create_positions();
    entity_create_velocities();
    entity_create_colliders();

    return next_entity_id;
}

void world_destroy_entity(Entity entity) {
    // TODO - add another array to world that tracks 'free' entity ids / array slots
    //   then when creating a new entity, don't always increment world.num_entities,
    //   instead first check for any unused entity slots and return one of those if available,
    //   otherwise increment world.num_entities and add a new slot
}

void entity_add_name(Entity entity, NameStr name) {
    world.names.active[entity] = true;
    strcpy_s(world.names.name[entity].val, NAME_MAX_LEN, name.val);
}

void entity_add_position(Entity entity, u32 x, u32 y) {
    world.positions.active[entity] = true;
    world.positions.x[entity] = x;
    world.positions.y[entity] = y;
    world.positions.prev_x[entity] = x;
    world.positions.prev_y[entity] = y;
}

void entity_add_velocity(Entity entity, f32 vel_x, f32 vel_y, f32 friction, f32 gravity) {
    world.velocities.active[entity] = true;
    world.velocities.vel_x[entity] = vel_x;
    world.velocities.vel_y[entity] = vel_y;
    world.velocities.remainder_x[entity] = 0;
    world.velocities.remainder_y[entity] = 0;
    world.velocities.friction[entity] = friction;
    world.velocities.gravity[entity] = gravity;
}

// TODO - add convenience functions for initializing different types of colliders
//   might only need a 'no width/height' version and a 'no radius' version
//   since they can be calculated from each other
void entity_add_collider(Entity entity, u32 offset_x, u32 offset_y, u32 width, u32 height, u32 radius) {
    world.collider_shapes.active[entity] = true;
    world.collider_shapes.offset_x[entity] = offset_x;
    world.collider_shapes.offset_y[entity] = offset_y;
    world.collider_shapes.width[entity] = width;
    world.collider_shapes.height[entity] = height;
    world.collider_shapes.radius[entity] = radius;
}

// -----------------------------------------------------------------------------
// Internal function implementations

internal void entity_create_names() {
    arrput(world.names.active, false);
    arrput(world.names.name, NAME_EMPTY);
}

internal void entity_create_positions() {
    arrput(world.positions.active, false);
    arrput(world.positions.x, 0);
    arrput(world.positions.y, 0);
    arrput(world.positions.prev_x, 0);
    arrput(world.positions.prev_y, 0);
}

internal void entity_create_velocities() {
    arrput(world.velocities.active, false);
    arrput(world.velocities.vel_x, 0);
    arrput(world.velocities.vel_y, 0);
    arrput(world.velocities.remainder_x, 0);
    arrput(world.velocities.remainder_y, 0);
    arrput(world.velocities.friction, 0);
    arrput(world.velocities.gravity, 0);
}

internal void entity_create_colliders() {
    arrput(world.collider_shapes.active, false);
    arrput(world.collider_shapes.offset_x, 0);
    arrput(world.collider_shapes.offset_y, 0);
    arrput(world.collider_shapes.width, 0);
    arrput(world.collider_shapes.height, 0);
    arrput(world.collider_shapes.radius, 0);
}

internal void entity_cleanup_names() {
    for (u32 i = 0; i < world.num_entities; ++i) {
        arrfree(world.names.active);
        arrfree(world.names.name);
    }
}

internal void entity_cleanup_positions() {
    for (u32 i = 0; i < world.num_entities; ++i) {
        arrfree(world.positions.active);
        arrfree(world.positions.x);
        arrfree(world.positions.y);
        arrfree(world.positions.prev_x);
        arrfree(world.positions.prev_y);
    }
}

internal void entity_cleanup_velocities() {
    for (u32 i = 0; i < world.num_entities; ++i) {
        arrfree(world.velocities.active);
        arrfree(world.velocities.vel_x);
        arrfree(world.velocities.vel_y);
        arrfree(world.velocities.remainder_x);
        arrfree(world.velocities.remainder_y);
    }
}

internal void entity_cleanup_colliders() {
    for (u32 i = 0; i < world.num_entities; ++i) {
        arrfree(world.collider_shapes.active);
        arrfree(world.collider_shapes.offset_x);
        arrfree(world.collider_shapes.offset_y);
        arrfree(world.collider_shapes.width);
        arrfree(world.collider_shapes.height);
        arrfree(world.collider_shapes.radius);
    }
}

typedef struct {
    Entity entity;
    NameStr *name;
    i32 x;
    i32 y;
    i32 prev_x;
    i32 prev_y;
    f32 vel_x;
    f32 vel_y;
    f32 remainder_x;
    f32 remainder_y;
    f32 friction;
    f32 gravity;
    i32 offset_x;
    i32 offset_y;
    u32 width;
    u32 height;
    u32 radius;
} EntityData;

internal void world_log() {
    TraceLog(LOG_DEBUG, "world: %d entities", arrlen(world.names.name));
    for (u32 i = 0; i < world.num_entities; ++i) {
        EntityData e = {0};
        e.entity = i;

        if (world.names.active[i]) {
            e.name = &world.names.name[i];
        }
        if (world.positions.active[i]) {
            e.x = world.positions.x[i];
            e.y = world.positions.y[i];
            e.prev_x = world.positions.prev_x[i];
            e.prev_y = world.positions.prev_y[i];
        }
        if (world.velocities.active[i]) {
            e.vel_x = world.velocities.vel_x[i];
            e.vel_y = world.velocities.vel_y[i];
            e.remainder_x = world.velocities.remainder_x[i];
            e.remainder_y = world.velocities.remainder_y[i];
            e.friction = world.velocities.friction[i];
            e.gravity = world.velocities.gravity[i];
        }
        if (world.collider_shapes.active[i]) {
            e.offset_x = world.collider_shapes.offset_x[i];
            e.offset_y = world.collider_shapes.offset_y[i];
            e.width = world.collider_shapes.width[i];
            e.height = world.collider_shapes.height[i];
            e.radius = world.collider_shapes.radius[i];
        }

        TraceLog(LOG_DEBUG, "Entity %d: name: '%s', pos: (%d, %d), prev_pos: (%d, %d), vel: (%f, %f), remainder: (%f, %f), friction: %f, gravity: %f, collider: (%d, %d, %d, %d, %d)\n",
                 e.entity, e.name->val, e.x, e.y, e.prev_x, e.prev_y, e.vel_x, e.vel_y, e.remainder_x, e.remainder_y, e.friction, e.gravity, e.offset_x, e.offset_y, e.width, e.height, e.radius);
    }
}
