#pragma once

#include "orbit_camera.h"

struct AppState
{
    OrbitCamera camera;
    int picked_face = -1;
    float tint[3] = { 1.0f, 1.0f, 1.0f };
};
