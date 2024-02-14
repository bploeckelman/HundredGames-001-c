#include "game.h"

internal bool entity_move_x(Entity entity, f32 amount);
internal bool entity_move_y(Entity entity, f32 amount);

internal bool entities_overlap(Entity a, Entity b, int offset_x, int offset_y);
internal void entities_resolve_collision(Entity a, Entity b);

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
        bool has_position = entity_has_components(i, COMPONENT_POSITION);
        bool has_velocity = entity_has_components(i, COMPONENT_MOVEMENT);
        bool has_collider = entity_has_components(i, COMPONENT_COLLIDER);

        if (has_position) {
            world.positions.prev_x[i] = world.positions.x[i];
            world.positions.prev_y[i] = world.positions.y[i];
        }

        f32 move_x = 0;
        f32 move_y = 0;
        if (has_velocity) {
            if (world.movements.friction[i] > 0) {
                world.movements.vel_x[i] = calc_approach(world.movements.vel_x[i], 0, world.movements.friction[i] * dt);
                world.movements.vel_y[i] = calc_approach(world.movements.vel_y[i], 0, world.movements.friction[i] * dt);
            }

            // TODO - set gravity direction, for now just apply to y
            if (world.movements.gravity[i] != 0) {
                world.movements.vel_y[i] += world.movements.gravity[i] * dt;
            }

            f32 total_move_x = world.movements.remainder_x[i] + world.movements.vel_x[i] * dt;
            f32 total_move_y = world.movements.remainder_y[i] + world.movements.vel_y[i] * dt;
            move_x = (i32) total_move_x;
            move_y = (i32) total_move_y;
            world.movements.remainder_x[i] = total_move_x - move_x;
            world.movements.remainder_y[i] = total_move_y - move_y;
        }

        if (has_position) {
            entity_move_x(i, move_x);
            entity_move_y(i, move_y);
        }

        // TODO - update colliders
        if (has_collider) {
            for (u32 j = 0; j < world.num_entities; j++) {
                if (i == j) continue;

                bool other_has_collider = entity_has_components(j, COMPONENT_COLLIDER);
                if (other_has_collider) {
                    if (entities_overlap(i, j, 0, 0)) {
                        entities_resolve_collision(i, j);
                    }
                }
            }
        }
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
    world.infos.components[entity] |= COMPONENT_NAME;

    strcpy_s(world.names.name[entity].val, NAME_MAX_LEN, name.val);
}

void entity_add_position(Entity entity, u32 x, u32 y) {
    world.infos.components[entity] |= COMPONENT_POSITION;

    world.positions.x[entity] = x;
    world.positions.y[entity] = y;
    world.positions.prev_x[entity] = x;
    world.positions.prev_y[entity] = y;
}

void entity_add_velocity(Entity entity, f32 vel_x, f32 vel_y, f32 friction, f32 gravity) {
    world.infos.components[entity] |= COMPONENT_MOVEMENT;

    world.movements.vel_x[entity] = vel_x;
    world.movements.vel_y[entity] = vel_y;
    world.movements.remainder_x[entity] = 0;
    world.movements.remainder_y[entity] = 0;
    world.movements.friction[entity] = friction;
    world.movements.gravity[entity] = gravity;
}

void entity_add_collider_rect(Entity entity, CollisionMask mask, u32 offset_x, u32 offset_y, u32 width, u32 height) {
    world.infos.components[entity] |= COMPONENT_COLLIDER;

    world.colliders.offset_x[entity] = offset_x;
    world.colliders.offset_y[entity] = offset_y;
    world.colliders.width[entity] = width;
    world.colliders.height[entity] = height;
    world.colliders.radius[entity] = calc_max(width, height) / 2;
    world.colliders.shape[entity] = SHAPE_RECT;
    world.colliders.mask[entity] = mask;
    world.colliders.on_hit_x[entity] = NULL;
    world.colliders.on_hit_y[entity] = NULL;
}

void entity_add_collider_circ(Entity entity, CollisionMask mask, u32 offset_x, u32 offset_y, u32 radius) {
    world.infos.components[entity] |= COMPONENT_COLLIDER;

    world.colliders.offset_x[entity] = offset_x;
    world.colliders.offset_y[entity] = offset_y;
    world.colliders.width[entity] = 2 * radius;
    world.colliders.height[entity] = 2 * radius;
    world.colliders.radius[entity] = radius;
    world.colliders.shape[entity] = SHAPE_CIRC;
    world.colliders.mask[entity] = mask;
    world.colliders.on_hit_x[entity] = NULL;
    world.colliders.on_hit_y[entity] = NULL;
}

// -----------------------------------------------------------------------------
// Internal implementation

internal bool entities_overlap(Entity a, Entity b, int offset_x, int offset_y) {
    i32 a_x = world.positions.x[a] + world.colliders.offset_x[a] + offset_x;
    i32 a_y = world.positions.y[a] + world.colliders.offset_y[a] + offset_y;
    i32 b_x = world.positions.x[b] + world.colliders.offset_x[b];
    i32 b_y = world.positions.y[b] + world.colliders.offset_y[b];

    switch (world.colliders.shape[a]) {
        case SHAPE_CIRC: {
            i32 a_r = world.colliders.radius[a];
            switch (world.colliders.shape[b]) {
                case SHAPE_CIRC: return circ_circ_overlaps(a_x, a_y, a_r, b_x, b_y, world.colliders.radius[b]);
                case SHAPE_RECT: return circ_rect_overlaps(a_x, a_y, a_r, b_x, b_y, world.colliders.width[b], world.colliders.height[b]);
                case SHAPE_NONE:
                default: break;
            }
        } break;
        case SHAPE_RECT: {
            i32 a_w = world.colliders.width[a];
            i32 a_h = world.colliders.height[a];
            switch (world.colliders.shape[b]) {
                case SHAPE_CIRC: return circ_rect_overlaps(b_x, b_y, world.colliders.radius[b], a_x, a_y, a_w, a_h);
                case SHAPE_RECT: return rect_rect_overlaps(a_x, a_y, a_w, a_h, b_x, b_y, world.colliders.width[b], world.colliders.height[b]);
                case SHAPE_NONE:
                default: break;
            }
        } break;

        case SHAPE_NONE:
        default: break;
    }

    return false;
}

internal void entities_resolve_collision(Entity a, Entity b) {
    switch (world.colliders.shape[a]) {
        case SHAPE_CIRC: {
            switch (world.colliders.shape[b]) {
                case SHAPE_CIRC: circ_circ_resolve(a, b); break;
                case SHAPE_RECT: circ_rect_resolve(a, b); break;
                case SHAPE_NONE:
                default: break;
            }
        } break;
        case SHAPE_RECT: {
            switch (world.colliders.shape[b]) {
                case SHAPE_CIRC: circ_rect_resolve(b, a); break;
                case SHAPE_RECT: rect_rect_resolve(a, b); break;
                case SHAPE_NONE:
                default: break;
            }
        } break;

        case SHAPE_NONE:
        default: break;
    }
}

internal void circ_circ_resolve(Entity entity, Entity collided_with) {
    f32 dx = world.positions.x[entity] - world.positions.x[collided_with];
    f32 dy = world.positions.y[entity] - world.positions.y[collided_with];
    f32 distance = sqrtf(dx * dx + dy * dy);
    dx /= distance;
    dy /= distance;

    f32 overlap = (world.colliders.radius[entity] + world.colliders.radius[collided_with]) - distance;
    world.positions.x[entity] -= dx * overlap / 2;
    world.positions.y[entity] -= dy * overlap / 2;
    world.positions.x[collided_with] += dx * overlap / 2;
    world.positions.y[collided_with] += dy * overlap / 2;

    // TODO - resolve velocities, just invert them for now
    world.movements.vel_x[entity] *= -1;
    world.movements.vel_y[entity] *= -1;
    world.movements.vel_x[collided_with] *= -1;
    world.movements.vel_y[collided_with] *= -1;
}

internal void circ_rect_resolve(Entity entity, Entity collided_with) {
    f32 nearest_x = calc_clamp(world.positions.x[entity], world.positions.x[collided_with], world.positions.x[collided_with] + world.colliders.width[collided_with]);
    f32 nearest_y = calc_clamp(world.positions.y[entity], world.positions.y[collided_with], world.positions.y[collided_with] + world.colliders.height[collided_with]);
    f32 dx = nearest_x - world.positions.x[entity];
    f32 dy = nearest_y - world.positions.y[entity];
    f32 distance = sqrtf(dx * dx + dy * dy);
    f32 overlap = world.colliders.radius[entity] - distance;
    dx /= distance;
    dy /= distance;

    world.positions.x[entity] -= dx * overlap;
    world.positions.y[entity] -= dy * overlap;

    // TODO - resolve velocities, just invert the circle for now
    world.movements.vel_x[entity] *= -1;
    world.movements.vel_y[entity] *= -1;
}

internal void rect_rect_resolve(Entity entity, Entity collided_with) {

}

internal Entity world_check_collisions(Entity entity, u32 mask, int offset_x, int offset_y) {
    Colliders *colliders = &world.colliders;

    for (u32 other = 0; other < world.num_entities; ++other) {
        bool is_different = (other != entity);
        bool is_masked = (colliders->mask[other] & mask) == mask;
        bool this_has_collider = entity_has_components(entity, COMPONENT_COLLIDER);
        bool that_has_collider = entity_has_components(other, COMPONENT_COLLIDER);

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

internal void world_resolve_collision(Entity a, Entity b) {
    // TODO - resolve collision
}

internal bool entity_move_x(Entity entity, f32 amount) {
    if (entity_has_components(entity, COMPONENT_COLLIDER)) {
        i32 sign = calc_sign(amount);

        while (amount != 0) {
            Entity would_collide_with = world_check_collisions(entity, MASK_BOUNDS, sign, 0);
            if (would_collide_with != ENTITY_NONE) {
                OnHitFunc on_hit = world.colliders.on_hit_x[entity];
                if (on_hit) {
                    on_hit(entity, would_collide_with);
                } else {
                    // stop
                    world.movements.vel_x[entity] = 0;
                    world.movements.remainder_x[entity] = 0;
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
    if (entity_has_components(entity, COMPONENT_COLLIDER)) {
        i32 sign = calc_sign(amount);

        while (amount != 0) {
            Entity would_collide_with = world_check_collisions(entity, MASK_BOUNDS, 0, sign);
            if (would_collide_with != ENTITY_NONE) {
                OnHitFunc on_hit = world.colliders.on_hit_y[entity];
                if (on_hit) {
                    on_hit(entity, would_collide_with);
                } else {
                    // stop
                    world.movements.vel_y[entity] = 0;
                    world.movements.remainder_y[entity] = 0;
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
    arrput(world.infos.components, COMPONENT_NONE);
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
    arrput(world.movements.vel_x, 0);
    arrput(world.movements.vel_y, 0);
    arrput(world.movements.remainder_x, 0);
    arrput(world.movements.remainder_y, 0);
    arrput(world.movements.friction, 0);
    arrput(world.movements.gravity, 0);
}

internal void entity_create_colliders() {
    arrput(world.colliders.offset_x, 0);
    arrput(world.colliders.offset_y, 0);
    arrput(world.colliders.width, 0);
    arrput(world.colliders.height, 0);
    arrput(world.colliders.radius, 0);
    arrput(world.colliders.shape, SHAPE_NONE);
    arrput(world.colliders.mask, MASK_NONE);
    arrput(world.colliders.on_hit_x, NULL);
    arrput(world.colliders.on_hit_y, NULL);
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
    arrfree(world.movements.vel_x);
    arrfree(world.movements.vel_y);
    arrfree(world.movements.remainder_x);
    arrfree(world.movements.remainder_y);
}

internal void entity_cleanup_colliders() {
    arrfree(world.colliders.offset_x);
    arrfree(world.colliders.offset_y);
    arrfree(world.colliders.width);
    arrfree(world.colliders.height);
    arrfree(world.colliders.radius);
    arrfree(world.colliders.shape);
    arrfree(world.colliders.mask);
    arrfree(world.colliders.on_hit_x);
    arrfree(world.colliders.on_hit_y);
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

        if (entity_has_components(i, COMPONENT_NAME)) {
            e.name = &world.names.name[i];
        }
        if (entity_has_components(i, COMPONENT_POSITION)) {
            e.x = world.positions.x[i];
            e.y = world.positions.y[i];
            e.prev_x = world.positions.prev_x[i];
            e.prev_y = world.positions.prev_y[i];
        }
        if (entity_has_components(i, COMPONENT_MOVEMENT)) {
            e.vel_x = world.movements.vel_x[i];
            e.vel_y = world.movements.vel_y[i];
            e.remainder_x = world.movements.remainder_x[i];
            e.remainder_y = world.movements.remainder_y[i];
            e.friction = world.movements.friction[i];
            e.gravity = world.movements.gravity[i];
        }
        if (entity_has_components(i, COMPONENT_COLLIDER)) {
            e.offset_x = world.colliders.offset_x[i];
            e.offset_y = world.colliders.offset_y[i];
            e.width = world.colliders.width[i];
            e.height = world.colliders.height[i];
            e.radius = world.colliders.radius[i];
        }

        TraceLog(LOG_INFO, "Entity %d (in_use: %d, active: %d, components: %#x): name: '%s', pos: (%d, %d), prev_pos: (%d, %d), vel: (%.2f, %.2f), remainder: (%.2f, %.2f), friction: %.2f, gravity: %.2f, collider: (%d, %d, %d, %d, %d)",
                 e.entity, e.in_use, e.active, e.components, e.name->val, e.x, e.y, e.prev_x, e.prev_y, e.vel_x, e.vel_y, e.remainder_x, e.remainder_y, e.friction, e.gravity, e.offset_x, e.offset_y, e.width, e.height, e.radius);
    }

    TraceLog(LOG_INFO, "--------------------------------------\n");
}
