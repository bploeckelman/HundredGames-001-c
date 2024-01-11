#ifndef PHYSICS_H
#define PHYSICS_H

#include "common.h"

#include "box2d/box2d.h"

typedef struct Physics {
    b2WorldId world;

    struct Bodies {
        b2BodyId top;
        b2BodyId bottom;
        b2BodyId left;
        b2BodyId right;
        b2BodyId ball;
    } bodies;
} Physics;

extern Physics physics;

void InitPhysics(struct Window window);

void UpdatePhysics();

void DebugDrawPhysics();

void ClosePhysics();

#endif //PHYSICS_H
