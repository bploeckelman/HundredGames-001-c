#include "game.h"

internal void ecs_cleanup_names();
internal void ecs_cleanup_positions();
internal void ecs_cleanup_velocities();
internal void ecs_cleanup_colliders();

internal void ecs_create_entity_names();
internal void ecs_create_entity_positions();
internal void ecs_create_entity_velocities();
internal void ecs_create_entity_colliders();


void ecs_init() {
    world = (World) {0};
}

void ecs_update() {
    // TODO
}

void ecs_cleanup() {
    ecs_cleanup_names();
    ecs_cleanup_positions();
    ecs_cleanup_velocities();
    ecs_cleanup_colliders();
    world = (World) {0};
}

Entity ecs_create_entity() {
    // return the next unused entity id:
    // since world.num_entities is initialized to 0 this is a pre-increment operation,
    // that way '0' can be retained as a sentinel value for 'no entity'
    u32 next_entity_id = ++world.num_entities;

    // TODO - scan for free slots first, if there is one, pass the index into the `ecs_create_entity_<component>()` functions
    //  to zero out the arrays for that entity's slot, see comment in ecs_destroy_entity()

    // add an 'empty' element to each component array for the new entity
    ecs_create_entity_names();
    ecs_create_entity_positions();
    ecs_create_entity_velocities();
    ecs_create_entity_colliders();

    return next_entity_id;
}

void ecs_destroy_entity(Entity entity) {
    // TODO - add another array to world that tracks 'free' entity ids / array slots
    //   then when creating a new entity, don't always increment world.num_entities,
    //   instead first check for any unused entity slots and return one of those if available,
    //   otherwise increment world.num_entities and add a new slot
}

void ecs_add_name(Entity entity, NameStr name) {
    world.names.active[entity] = true;
    world.names.name[entity] = name;
}

void ecs_add_position(Entity entity, u32 x, u32 y) {
    world.positions.active[entity] = true;
    world.positions.x[entity] = x;
    world.positions.y[entity] = y;
    world.positions.prev_x[entity] = x;
    world.positions.prev_y[entity] = y;
}

void ecs_add_velocity(Entity entity, f32 vel_x, f32 vel_y) {
    world.velocities.active[entity] = true;
    world.velocities.vel_x[entity] = vel_x;
    world.velocities.vel_y[entity] = vel_y;
    world.velocities.remainder_x[entity] = 0;
    world.velocities.remainder_y[entity] = 0;
}

// TODO - add some convenience functions for initializing different types of colliders
void ecs_add_collider(Entity entity, u32 offset_x, u32 offset_y, u32 width, u32 height, u32 radius) {
    world.collider_shapes.active[entity] = true;
    world.collider_shapes.offset_x[entity] = offset_x;
    world.collider_shapes.offset_y[entity] = offset_y;
    world.collider_shapes.width[entity] = width;
    world.collider_shapes.height[entity] = height;
    world.collider_shapes.radius[entity] = radius;
}

// -----------------------------------------------------------------------------
// Internal function implementations

internal void ecs_cleanup_names() {
    for (u32 i = 0; i < world.num_entities; ++i) {
        arrfree(world.names.active);
        arrfree(world.names.name);
    }
}

internal void ecs_cleanup_positions() {
    for (u32 i = 0; i < world.num_entities; ++i) {
        arrfree(world.positions.active);
        arrfree(world.positions.x);
        arrfree(world.positions.y);
        arrfree(world.positions.prev_x);
        arrfree(world.positions.prev_y);
    }
}

internal void ecs_cleanup_velocities() {
    for (u32 i = 0; i < world.num_entities; ++i) {
        arrfree(world.velocities.active);
        arrfree(world.velocities.vel_x);
        arrfree(world.velocities.vel_y);
        arrfree(world.velocities.remainder_x);
        arrfree(world.velocities.remainder_y);
    }
}

internal void ecs_cleanup_colliders() {
    for (u32 i = 0; i < world.num_entities; ++i) {
        arrfree(world.collider_shapes.active);
        arrfree(world.collider_shapes.offset_x);
        arrfree(world.collider_shapes.offset_y);
        arrfree(world.collider_shapes.width);
        arrfree(world.collider_shapes.height);
        arrfree(world.collider_shapes.radius);
    }
}

internal void ecs_create_entity_names() {
    arrput(world.names.active, false);
    arrput(world.names.name, NAME_EMPTY);
}

internal void ecs_create_entity_positions() {
    arrput(world.positions.active, false);
    arrput(world.positions.x, 0);
    arrput(world.positions.y, 0);
    arrput(world.positions.prev_x, 0);
    arrput(world.positions.prev_y, 0);
}

internal void ecs_create_entity_velocities() {
    arrput(world.velocities.active, false);
    arrput(world.velocities.vel_x, 0);
    arrput(world.velocities.vel_y, 0);
    arrput(world.velocities.remainder_x, 0);
    arrput(world.velocities.remainder_y, 0);
}

internal void ecs_create_entity_colliders() {
    arrput(world.collider_shapes.active, false);
    arrput(world.collider_shapes.offset_x, 0);
    arrput(world.collider_shapes.offset_y, 0);
    arrput(world.collider_shapes.width, 0);
    arrput(world.collider_shapes.height, 0);
    arrput(world.collider_shapes.radius, 0);
}
