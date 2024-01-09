#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "dark/style_dark.h"

#include "box2d/box2d.h"

#include "common.h"

#define GLSL_VERSION 330

static State state = {
    .window = {
        .width = 1280,
        .height = 720,
        .title = "Prong"
    },
};

static void Init();

static void Update();

static void DrawFrame();

static void Shutdown();

// ----------------------------------------------------------------------------

int main() {
    Init();
    while (!WindowShouldClose()) {
        Update();
        DrawFrame();
    }
    Shutdown();
    return 0;
}

// ----------------------------------------------------------------------------

static void Init() {
    // init raylib
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(state.window.width, state.window.height, state.window.title);
    SetTargetFPS(60);

    // init raygui
    GuiLoadStyleDark();

    // init box2d
    // TODO - move to 'init game state' section
    state.scene.physics.world_id = b2CreateWorld(&b2_defaultWorldDef);

    const b2BodyDef ground_def = b2_defaultBodyDef;
    state.scene.physics.ground_body_id = b2CreateBody(state.scene.physics.world_id, &ground_def);
    const b2Segment ground_segment = {
        {-state.window.width / 2, -state.window.height / 2 + 20},
        {state.window.width / 2, -state.window.height / 2 + 20}
    };
    const b2ShapeDef ground_shape = b2_defaultShapeDef;
    b2CreateSegmentShape(state.scene.physics.ground_body_id, &ground_shape, &ground_segment);

    b2BodyDef ball_def = b2_defaultBodyDef;
    ball_def.type = b2_dynamicBody;
    state.scene.physics.ball_body_id = b2CreateBody(state.scene.physics.world_id, &ball_def);
    const b2Circle ball_circle = { {0, 0}, 25 };
    b2ShapeDef ball_shape = b2_defaultShapeDef;
    ball_shape.restitution = 0.8f;
    ball_shape.density = 5.0f;
    b2CreateCircleShape(state.scene.physics.ball_body_id, &ball_shape, &ball_circle);

    // init game state
    state.camera = (Camera2D){
        .target = (Vector2){0.0f, 0.0f},
        .offset = (Vector2){0.0f, 0.0f},
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    state.render_texture = LoadRenderTexture(state.window.width, state.window.height);
}

static void Update() {
    // update physics bodies
    const float time = 0.1f; // fixed time step
    b2World_Step(state.scene.physics.world_id, time, 8, 3);

    // update camera
    state.camera.target = (Vector2){0, 0};
    state.camera.offset = (Vector2){state.window.width / 2, state.window.height / 2};
    state.camera.rotation = 0.0f;
    state.camera.zoom = 1.0f;
}

static void DrawFrame() {
    // draw world to render texture
    BeginTextureMode(state.render_texture);
    ClearBackground(RAYWHITE);

    BeginMode2D(state.camera);

    // TODO - setup b2DebugDraw instance with callbacks for each shape that use raylib drawing functions
    // b2World_Draw(scene.world_id, &b2_debug_draw);

    const b2ShapeId shape = b2Body_GetFirstShape(state.scene.physics.ground_body_id);
    const b2Vec2 p = b2Body_GetPosition(state.scene.physics.ground_body_id);
    const b2Segment* segment = b2Shape_GetSegment(shape);
    DrawLineV(
        (Vector2){segment->point1.x - p.x, segment->point1.y - p.y},
        (Vector2){segment->point2.x - p.x, segment->point2.y - p.y},
        RED);

    const b2ShapeId ball_shape = b2Body_GetFirstShape(state.scene.physics.ball_body_id);
    const b2Vec2 ball_pos = b2Body_GetPosition(state.scene.physics.ball_body_id);
    const b2Circle *circle = b2Shape_GetCircle(ball_shape);
    DrawCircleGradient(
        ball_pos.x - circle->point.x,
        ball_pos.y - circle->point.y,
        circle->radius,
        RED, ORANGE);

    EndMode2D();

    EndTextureMode();

    // draw render texture to screen
    BeginDrawing();
    ClearBackground(BLACK);

    DrawTextureEx(state.render_texture.texture, (Vector2){0, 0}, 0, 1, WHITE);
    DrawText(state.window.title, 10, 10, 20, DARKGRAY);

    EndDrawing();
}

static void Shutdown() {
    UnloadRenderTexture(state.render_texture);

    // NOTE - destroying the world destroys all the bodies attached to it
    b2DestroyWorld(state.scene.physics.world_id);

    CloseWindow();
}
