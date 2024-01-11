#include "../include/physics.h"

#include "raylib.h" // for debug drawing
#include "box2d/debug_draw.h"

// Public data ----------------------------------------------------------------

Physics physics = {0};

// Private data ---------------------------------------------------------------

b2DebugDraw physics_debug_draw;

static void InitPhysicsDebugDraw();

// Public implementation ------------------------------------------------------

void InitPhysics(struct Window window) {
    InitPhysicsDebugDraw();

    // create the physics world
    b2WorldDef world_def = b2_defaultWorldDef;
    // defaults are fine...
    physics.world = b2CreateWorld(&world_def);

    // create the physics bodies
    b2BodyDef ball_def = b2_defaultBodyDef;
    ball_def.type = b2_dynamicBody;
    physics.bodies.ball = b2CreateBody(physics.world, &ball_def);

    b2BodyDef wall_def = b2_defaultBodyDef;
    wall_def.type = b2_staticBody;
    physics.bodies.top = b2CreateBody(physics.world, &wall_def);
    physics.bodies.bottom = b2CreateBody(physics.world, &wall_def);
    physics.bodies.left = b2CreateBody(physics.world, &wall_def);
    physics.bodies.right = b2CreateBody(physics.world, &wall_def);

    // create a shape for the physics ball body
    b2Circle ball_circle = {{0, 0}, 25};
    b2ShapeDef ball_shape_def = b2_defaultShapeDef;
    ball_shape_def.restitution = 0.9f;
    ball_shape_def.density = 0.1f;
    b2CreateCircleShape(physics.bodies.ball, &ball_shape_def, &ball_circle);

    // create shapes for the physics wall bodies
    const float margin = 40;
    const float horiz_width = window.width - margin * 2;
    const float vert_height = window.height - margin * 2;
    const b2Segment segment_top = {
        {-horiz_width / 2, vert_height / 2},
        {horiz_width / 2, vert_height / 2}
    };
    const b2Segment segment_bottom = {
        {-horiz_width / 2, -vert_height / 2},
        {horiz_width / 2, -vert_height / 2}
    };
    const b2Segment segment_left = {
        {-horiz_width / 2, -vert_height / 2},
        {-horiz_width / 2, vert_height / 2}
    };
    const b2Segment segment_right = {
        {horiz_width / 2, -vert_height / 2},
        {horiz_width / 2, vert_height / 2}
    };

    b2ShapeDef wall_shape_def = b2_defaultShapeDef;
    // defaults are fine for now...
    b2CreateSegmentShape(physics.bodies.top, &wall_shape_def, &segment_top);
    b2CreateSegmentShape(physics.bodies.bottom, &wall_shape_def, &segment_bottom);
    b2CreateSegmentShape(physics.bodies.left, &wall_shape_def, &segment_left);
    b2CreateSegmentShape(physics.bodies.right, &wall_shape_def, &segment_right);
}

void UpdatePhysics() {
    // update the physics world
    // TODO - https://gafferongames.com/post/fix_your_timestep/
    const float fixed_time_step = 1.0f / 60.0f;
    float dt = GetFrameTime();
    while (dt > 0.0f) {
        b2World_Step(physics.world, fixed_time_step, 8, 3);
        dt -= fixed_time_step;
    }
}

void DebugDrawPhysics() {
    b2World_Draw(physics.world, &physics_debug_draw);
}

void ClosePhysics() {
    // destroying the world also destroys anything attached to it
    b2DestroyWorld(physics.world);
}

// Private implementation -----------------------------------------------------

static Color ConvertB2ColorToRaylib(const b2Color color) {
    return (Color){
        (unsigned char)(color.r * 255),
        (unsigned char)(color.g * 255),
        (unsigned char)(color.b * 255),
        (unsigned char)(color.a * 255)
    };
}

/// Draw a closed polygon provided in CCW order.
static void PhysDrawPolygon(const b2Vec2* vertices, int vertexCount, b2Color color, void* context) {
    const Color raylib_color = ConvertB2ColorToRaylib(color);
    for (int i = 0; i < vertexCount - 1; ++i) {
        DrawLine(
            vertices[i].x,
            vertices[i].y,
            vertices[i + 1].x,
            vertices[i + 1].y,
            raylib_color);
    }
    // close it
    DrawLine(
        vertices[vertexCount - 1].x,
        vertices[vertexCount - 1].y,
        vertices[0].x,
        vertices[0].y,
        raylib_color);
}

/// Draw a solid closed polygon provided in CCW order.
static void PhysDrawSolidPolygon(const b2Vec2* vertices, int vertexCount, b2Color color, void* context) {
    // TODO - DrawTexturedPolygon is probably the least hassle way to do this, but we need a solid white pixel texture
}

/// Draw a rounded polygon provided in CCW order.
static void DrawRoundedPolygon(const b2Vec2* vertices, int vertexCount, float radius, b2Color lineColor,
                               b2Color fillColor, void* context) {
    // TODO
}

/// Draw a circle.
/// TODO - not sure what 'axis' is for
static void PhysDrawCircle(b2Vec2 center, float radius, b2Color color, void* context) {
    DrawCircleLines(center.x, center.y, radius, ConvertB2ColorToRaylib(color));
}

/// Draw a solid circle.
/// TODO - not sure what 'axis' is for
static void PhysDrawSolidCircle(b2Vec2 center, float radius, b2Vec2 axis, b2Color color, void* context) {
    DrawCircle(center.x, center.y, radius, ConvertB2ColorToRaylib(color));
}

/// Draw a capsule.
static void PhysDrawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2Color color, void* context) {
    // TODO
}

/// Draw a solid capsule.
static void PhysDrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2Color color, void* context) {
    // TODO
}

/// Draw a line segment.
static void PhysDrawSegment(b2Vec2 p1, b2Vec2 p2, b2Color color, void* context) {
    DrawLine(p1.x, p1.y, p2.x, p2.y, ConvertB2ColorToRaylib(color));
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
static void PhysDrawTransform(b2Transform xf, void* context) {
    const float axis_scale = 1.0f;
    const b2Vec2 p1 = xf.p;
    const b2Vec2 p2 = {
        p1.x + axis_scale * xf.q.s, // x axis (sin of rotation)
        p1.y + axis_scale * xf.q.c // y axis (cos of rotation)
    };
    DrawLine(p1.x, p1.y, p2.x, p2.y, RED);
    DrawLine(p1.x, p1.y, p2.x, p2.y, GREEN);
}

/// Draw a point.
static void PhysDrawPoint(b2Vec2 p, float size, b2Color color, void* context) {
    DrawCircle(p.x, p.y, size, ConvertB2ColorToRaylib(color));
}

/// Draw a string.
static void PhysDrawString(b2Vec2 p, const char* s, void* context) {
    // TODO - this is slightly more complicated so hold off for now
}

static void InitPhysicsDebugDraw() {
    physics_debug_draw = (b2DebugDraw){
        .DrawPolygon = PhysDrawPolygon,
        .DrawSolidPolygon = PhysDrawSolidPolygon,
        .DrawRoundedPolygon = DrawRoundedPolygon,
        .DrawCircle = PhysDrawCircle,
        .DrawSolidCircle = PhysDrawSolidCircle,
        .DrawCapsule = PhysDrawCapsule,
        .DrawSolidCapsule = PhysDrawSolidCapsule,
        .DrawSegment = PhysDrawSegment,
        .DrawTransform = PhysDrawTransform,
        .DrawPoint = PhysDrawPoint,
        .DrawString = PhysDrawString,
        .drawShapes = true,
        .drawJoints = true,
        .drawAABBs = false,
        .drawMass = false,
        .drawContacts = false,
        .drawGraphColors = false,
        .drawContactNormals = false,
        .drawContactImpulses = false,
        .drawFrictionImpulses = false,
        .context = NULL
    };
}
