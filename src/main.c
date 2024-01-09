#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "dark/style_dark.h"

// undefine crt stdlib.h min/max macros, they conflict with physac.h
#undef min
#undef max
#define PHYSAC_IMPLEMENTATION
#include "physac.h"

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

    // init physac
    InitPhysics();
    SetPhysicsGravity(0, -9.8f * 0.16f);
    //SetPhysicsTimeStep(1.0/60.0/100*1000);

    // init game state
    state.scene.physics = (struct Physics){
        .floor = CreatePhysicsBodyRectangle(
            (Vector2){0, -state.window.height / 2},
            state.window.width,
            50,
            10),
        .ball = CreatePhysicsBodyCircle(
            (Vector2){0, 0},
            30,
            10)
    };
    state.camera = (Camera2D){
        .target = (Vector2){0.0f, 0.0f},
        .offset = (Vector2){0.0f, 0.0f},
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    state.render_texture = LoadRenderTexture(state.window.width, state.window.height);

    // set floor physics body as static
    state.scene.physics.floor->enabled = false;
}

static void Update() {
    // update physics bodies
    // const int numBodies = GetPhysicsBodiesCount();
    // for (int i = 0; i < numBodies; i++) {
    //     const PhysicsBody body = GetPhysicsBody(i);
    //     if (body != NULL && body->enabled) {
    //         if (body == state.scene.physics.ball) {
    //             if (CheckCollisionCircleRec(
    //                     body->position,
    //                     body->shape.radius,
    //                     (Rectangle){
    //                         state.scene.physics.floor->position.x - state.window.width / 2,
    //                         state.scene.physics.floor->position.y - 50 / 2,
    //                         state.window.width,
    //                         50})) {
    //                 // reset ball position
    //                 body->position = (Vector2){0, state.window.height / 2};
    //             }
    //         }
    //     }
    // }

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

    const int numBodies = GetPhysicsBodiesCount();
    for (int i = 0; i < numBodies; i++) {
        const PhysicsBody body = GetPhysicsBody(i);
        if (body == NULL) {
            continue;
        }

        const int numVertices = GetPhysicsShapeVerticesCount(i);
        for (int curr = 0; curr < numVertices; curr++) {
            const int next = ((curr + 1) < numVertices) ? curr + 1 : 0;
            const Vector2 vertexA = GetPhysicsShapeVertex(body, curr);
            const Vector2 vertexB = GetPhysicsShapeVertex(body, next);
            DrawLineV(vertexA, vertexB, RED);
        }
    }

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

    ClosePhysics();

    CloseWindow();
}
