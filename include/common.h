#ifndef PRONG_COMMON_H
#define PRONG_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#include "box2d/box2d.h"

typedef float f32;
typedef double f64;
typedef uint8_t b8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// ----------------------------------------------------------------------------
// Game state data
// ----------------------------------------------------------------------------

typedef struct State {
    struct Window {
        int width;
        int height;
        char* title;
    } window;

    struct Scene {
        struct Physics {
            b2WorldId world_id;
            b2BodyId ground_body_id;
            b2BodyId ball_body_id;
        } physics;
    } scene;

    Camera2D camera;
    RenderTexture render_texture;
} State;

#endif //PRONG_COMMON_H
