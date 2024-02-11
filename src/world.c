#include "game.h"

internal bool entity_move_x(Entity entity, f32 amount);
internal bool entity_move_y(Entity entity, f32 amount);

internal void entity_create_infos();
internal void entity_create_names();
internal void entity_create_positions();
internal void entity_create_velocities();
internal void entity_create_colliders();

internal void entity_cleanup_infos();
internal void entity_cleanup_names();
internal void entity_cleanup_positions();
internal void entity_cleanup_velocities();
internal void entity_cleanup_colliders();

internal void world_log();

// -----------------------------------------------------------------------------
// Implementation

void world_init() {
    if (world.initialized) {
        world_cleanup();
    }

    world = (World) {0};
    world.initialized = true;

    // reserve the '0' entity id to represent 'no entity'
    world_create_entity();
    entity_add_name(0, (NameStr) {"ENTITY_NONE"});
}

void world_update(f32 dt) {
    if (!world.initialized) {
        world_init();
    }

    if (state.debug.log) {
        world_log();
    }

    for (u32 i = 0; i < world.num_entities; i++) {
        bool has_position = entity_has_components(i, COMP_POSITION);
        bool has_velocity = entity_has_components(i, COMP_VELOCITY);

        if (has_position) {
            world.positions.prev_x[i] = world.positions.x[i];
            world.positions.prev_y[i] = world.positions.y[i];
        }

        f32 move_x = 0;
        f32 move_y = 0;
        if (has_velocity) {
            if (world.velocities.friction[i] > 0) {
                world.velocities.x[i] = calc_approach(world.velocities.x[i], 0, world.velocities.friction[i] * dt);
                world.velocities.y[i] = calc_approach(world.velocities.y[i], 0, world.velocities.friction[i] * dt);
            }

            // TODO - set gravity direction, for now just apply to y
            if (world.velocities.gravity[i] != 0) {
                world.velocities.y[i] += world.velocities.gravity[i] * dt;
            }

            f32 total_move_x = world.velocities.remainder_x[i] + world.velocities.x[i] * dt;
            f32 total_move_y = world.velocities.remainder_y[i] + world.velocities.y[i] * dt;
            move_x = (i32) total_move_x;
            move_y = (i32) total_move_y;
            world.velocities.remainder_x[i] = total_move_x - move_x;
            world.velocities.remainder_y[i] = total_move_y - move_y;
        }

        if (has_position) {
            entity_move_x(i, move_x);
            entity_move_y(i, move_y);
        }

        // TODO - update colliders
    }
}

void world_cleanup() {
    if (world.initialized) {
        entity_cleanup_infos();
        entity_cleanup_names();
        entity_cleanup_positions();
        entity_cleanup_velocities();
        entity_cleanup_colliders();
    }
    world = (World) {0};
}

Entity world_create_entity() {
    // ensure that we have an initialized world before creating any entities
    if (!world.initialized) {
        world_init();
    }

    // return the next unused entity id
    u32 next_entity_id = world.num_entities++;

    // TODO - scan for available slots first, if there is one pass the index into `entity_add_<component>()`
    //  to zero out the arrays for that entity's slot, see comment in world_destroy_entity()

    // add an 'empty' element to each component array for the new entity
    entity_create_infos();
    entity_create_names();
    entity_create_positions();
    entity_create_velocities();
    entity_create_colliders();

    // mark this entity as in use and active
    world.infos.active[next_entity_id] = true;
    world.infos.in_use[next_entity_id] = true;

    return next_entity_id;
}

void world_destroy_entity(Entity entity) {
    // TODO -
    //   then when creating a new entity, don't always increment world.num_entities,
    //   instead first check for any unused entity slots and return one of those if available,
    //   otherwise increment world.num_entities and add a new slot
}

bool entity_has_components(Entity entity, ComponentMask mask) {
    bool is_invalid = (entity == ENTITY_NONE || entity >= world.num_entities); // TODO - equality too? double check
    bool is_unused = !world.infos.in_use[entity];
    if (is_invalid && is_unused) {
        return false;
    }

    return (world.infos.components[entity] & mask) == mask;
}

void entity_add_name(Entity entity, NameStr name) {
    world.infos.components[entity] |= COMP_NAME;

    strcpy_s(world.names.name[entity].val, NAME_MAX_LEN, name.val);
}

void entity_add_position(Entity entity, u32 x, u32 y) {
    world.infos.components[entity] |= COMP_POSITION;

    world.positions.x[entity] = x;
    world.positions.y[entity] = y;
    world.positions.prev_x[entity] = x;
    world.positions.prev_y[entity] = y;
}

void entity_add_velocity(Entity entity, f32 vel_x, f32 vel_y, f32 friction, f32 gravity) {
    world.infos.components[entity] |= COMP_VELOCITY;

    world.velocities.x[entity] = vel_x;
    world.velocities.y[entity] = vel_y;
    world.velocities.remainder_x[entity] = 0;
    world.velocities.remainder_y[entity] = 0;
    world.velocities.friction[entity] = friction;
    world.velocities.gravity[entity] = gravity;
}

void entity_add_collider_rect(Entity entity, CollisionMask mask, u32 offset_x, u32 offset_y, u32 width, u32 height) {
    world.infos.components[entity] |= COMP_COLLIDER;

    world.collider_shapes.offset_x[entity] = offset_x;
    world.collider_shapes.offset_y[entity] = offset_y;
    world.collider_shapes.width[entity] = width;
    world.collider_shapes.height[entity] = height;
    world.collider_shapes.radius[entity] = calc_max(width, height) / 2;
    world.collider_shapes.type[entity] = SHAPE_RECT;
    world.collider_shapes.mask[entity] = mask;
    world.collider_shapes.on_hit_x[entity] = NULL;
    world.collider_shapes.on_hit_y[entity] = NULL;
}

void entity_add_collider_circ(Entity entity, CollisionMask mask, u32 offset_x, u32 offset_y, u32 radius) {
    world.infos.components[entity] |= COMP_COLLIDER;

    world.collider_shapes.offset_x[entity] = offset_x;
    world.collider_shapes.offset_y[entity] = offset_y;
    world.collider_shapes.width[entity] = 2 * radius;
    world.collider_shapes.height[entity] = 2 * radius;
    world.collider_shapes.radius[entity] = radius;
    world.collider_shapes.type[entity] = SHAPE_CIRC;
    world.collider_shapes.mask[entity] = mask;
    world.collider_shapes.on_hit_x[entity] = NULL;
    world.collider_shapes.on_hit_y[entity] = NULL;
}

// -----------------------------------------------------------------------------
// Internal implementation

internal bool entities_overlap(Entity a, Entity b, int offset_x, int offset_y) {
    i32 a_x = world.positions.x[a] + world.collider_shapes.offset_x[a] + offset_x;
    i32 a_y = world.positions.y[a] + world.collider_shapes.offset_y[a] + offset_y;
    i32 b_x = world.positions.x[b] + world.collider_shapes.offset_x[b];
    i32 b_y = world.positions.y[b] + world.collider_shapes.offset_y[b];

    switch (world.collider_shapes.type[a]) {
        case SHAPE_CIRC: {
            i32 a_r = world.collider_shapes.radius[a];
            switch (world.collider_shapes.type[b]) {
                case SHAPE_CIRC: return circ_circ_overlaps(a_x, a_y, a_r, b_x, b_y, world.collider_shapes.radius[b]);
                case SHAPE_RECT: return circ_rect_overlaps(a_x, a_y, a_r, b_x, b_y, world.collider_shapes.width[b], world.collider_shapes.height[b]);
                case SHAPE_NONE:
                default: break;
            }
        } break;
        case SHAPE_RECT: {
            i32 a_w = world.collider_shapes.width[a];
            i32 a_h = world.collider_shapes.height[a];
            switch (world.collider_shapes.type[b]) {
                case SHAPE_CIRC: return circ_rect_overlaps(b_x, b_y, world.collider_shapes.radius[b], a_x, a_y, a_w, a_h);
                case SHAPE_RECT: return rect_rect_overlaps(a_x, a_y, a_w, a_h, b_x, b_y, world.collider_shapes.width[b], world.collider_shapes.height[b]);
                case SHAPE_NONE:
                default: break;
            }
        } break;

        case SHAPE_NONE:
        default: break;
    }

    return false;
}

internal Entity world_check_collisions(Entity entity, u32 mask, int offset_x, int offset_y) {
    ColliderShape *colliders = &world.collider_shapes;

    for (u32 other = 0; other < world.num_entities; ++other) {
        bool is_different = (other != entity);
        bool is_masked = (colliders->mask[other] & mask) == mask;
        bool this_has_collider = entity_has_components(entity, COMP_COLLIDER);
        bool that_has_collider = entity_has_components(other, COMP_COLLIDER);

        // skip entities that don't need to be processed
        if (!is_different || !is_masked || !this_has_collider || !that_has_collider) {
            continue;
        }

        if (entities_overlap(entity, other, offset_x, offset_y)) {
            return other;
        }
    }
    return ENTITY_NONE;
}

internal bool entity_move_x(Entity entity, f32 amount) {
    if (entity_has_components(entity, COMP_COLLIDER)) {
        i32 sign = calc_sign(amount);

        while (amount != 0) {
            Entity would_collide_with = world_check_collisions(entity, MASK_BOUNDS, sign, 0);
            if (would_collide_with != ENTITY_NONE) {
                OnHitFunc on_hit = world.collider_shapes.on_hit_x[entity];
                if (on_hit) {
                    on_hit(entity, would_collide_with);
                } else {
                    // stop
                    world.velocities.x[entity] = 0;
                    world.velocities.remainder_x[entity] = 0;
                }

                // moving any further would cause an overlap of colliders
                return true;
            }

            // won't collide, move one unit
            amount -= sign;
            world.positions.x[entity] += sign;
        }
    } else {
        // no collider, just move the full amount
        world.positions.x[entity] += amount;
    }

    // didn't hit anything
    return false;
}

internal bool entity_move_y(Entity entity, f32 amount) {
    if (entity_has_components(entity, COMP_COLLIDER)) {
        i32 sign = calc_sign(amount);

        while (amount != 0) {
            Entity would_collide_with = world_check_collisions(entity, MASK_BOUNDS, 0, sign);
            if (would_collide_with != ENTITY_NONE) {
                OnHitFunc on_hit = world.collider_shapes.on_hit_y[entity];
                if (on_hit) {
                    on_hit(entity, would_collide_with);
                } else {
                    // stop
                    world.velocities.y[entity] = 0;
                    world.velocities.remainder_y[entity] = 0;
                }

                // moving any further would cause an overlap of colliders
                return true;
            }

            // won't collide, move one unit
            amount -= sign;
            world.positions.y[entity] += sign;
        }
    } else {
        // no collider, just move the full amount
        world.positions.y[entity] += amount;
    }

    // didn't hit anything
    return false;
}

internal void entity_create_infos() {
    arrput(world.infos.active, false);
    arrput(world.infos.in_use, false);
    arrput(world.infos.components, COMP_NONE);
}

internal void entity_create_names() {
    arrput(world.names.name, NAME_EMPTY);
}

internal void entity_create_positions() {
    arrput(world.positions.x, 0);
    arrput(world.positions.y, 0);
    arrput(world.positions.prev_x, 0);
    arrput(world.positions.prev_y, 0);
}

internal void entity_create_velocities() {
    arrput(world.velocities.x, 0);
    arrput(world.velocities.y, 0);
    arrput(world.velocities.remainder_x, 0);
    arrput(world.velocities.remainder_y, 0);
    arrput(world.velocities.friction, 0);
    arrput(world.velocities.gravity, 0);
}

internal void entity_create_colliders() {
    arrput(world.collider_shapes.offset_x, 0);
    arrput(world.collider_shapes.offset_y, 0);
    arrput(world.collider_shapes.width, 0);
    arrput(world.collider_shapes.height, 0);
    arrput(world.collider_shapes.radius, 0);
    arrput(world.collider_shapes.type, SHAPE_NONE);
    arrput(world.collider_shapes.mask, MASK_NONE);
    arrput(world.collider_shapes.on_hit_x, NULL);
    arrput(world.collider_shapes.on_hit_y, NULL);
}

internal void entity_cleanup_infos() {
    arrfree(world.infos.active);
    arrfree(world.infos.in_use);
    arrfree(world.infos.components);
}

internal void entity_cleanup_names() {
    arrfree(world.names.name);
}

internal void entity_cleanup_positions() {
    arrfree(world.positions.x);
    arrfree(world.positions.y);
    arrfree(world.positions.prev_x);
    arrfree(world.positions.prev_y);
}

internal void entity_cleanup_velocities() {
    arrfree(world.velocities.x);
    arrfree(world.velocities.y);
    arrfree(world.velocities.remainder_x);
    arrfree(world.velocities.remainder_y);
}

internal void entity_cleanup_colliders() {
    arrfree(world.collider_shapes.offset_x);
    arrfree(world.collider_shapes.offset_y);
    arrfree(world.collider_shapes.width);
    arrfree(world.collider_shapes.height);
    arrfree(world.collider_shapes.radius);
    arrfree(world.collider_shapes.type);
    arrfree(world.collider_shapes.mask);
    arrfree(world.collider_shapes.on_hit_x);
    arrfree(world.collider_shapes.on_hit_y);
}

typedef struct {
    Entity entity;
    bool in_use;
    bool active;
    ComponentMask components;
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
    TraceLog(LOG_INFO, "world: %d entities", world.num_entities);
    TraceLog(LOG_INFO, "--------------------------------------");

    for (u32 i = 0; i < world.num_entities; ++i) {
        if (i == ENTITY_NONE) {
            continue;
        }

        EntityData e = {0};
        e.entity = i;

        e.in_use = world.infos.in_use[i];
        e.active = world.infos.active[i];
        e.components = world.infos.components[i];

        if (entity_has_components(i, COMP_NAME)) {
            e.name = &world.names.name[i];
        }
        if (entity_has_components(i, COMP_POSITION)) {
            e.x = world.positions.x[i];
            e.y = world.positions.y[i];
            e.prev_x = world.positions.prev_x[i];
            e.prev_y = world.positions.prev_y[i];
        }
        if (entity_has_components(i, COMP_VELOCITY)) {
            e.vel_x = world.velocities.x[i];
            e.vel_y = world.velocities.y[i];
            e.remainder_x = world.velocities.remainder_x[i];
            e.remainder_y = world.velocities.remainder_y[i];
            e.friction = world.velocities.friction[i];
            e.gravity = world.velocities.gravity[i];
        }
        if (entity_has_components(i, COMP_COLLIDER)) {
            e.offset_x = world.collider_shapes.offset_x[i];
            e.offset_y = world.collider_shapes.offset_y[i];
            e.width = world.collider_shapes.width[i];
            e.height = world.collider_shapes.height[i];
            e.radius = world.collider_shapes.radius[i];
        }

        TraceLog(LOG_INFO, "Entity %d (in_use: %d, active: %d, components: %#x): name: '%s', pos: (%d, %d), prev_pos: (%d, %d), vel: (%.2f, %.2f), remainder: (%.2f, %.2f), friction: %.2f, gravity: %.2f, collider: (%d, %d, %d, %d, %d)",
                 e.entity, e.in_use, e.active, e.components, e.name->val, e.x, e.y, e.prev_x, e.prev_y, e.vel_x, e.vel_y, e.remainder_x, e.remainder_y, e.friction, e.gravity, e.offset_x, e.offset_y, e.width, e.height, e.radius);
    }

    TraceLog(LOG_INFO, "--------------------------------------\n");
}
