#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <vector>
#include <vertex.h>

struct Window {
    int width;
    int height;
};

struct Projection {
    float fov;
    float near;
    float far;
};

struct Camera {
    Vertex3f position;
    Vertex3f lookAt;
    Vertex3f up;
    Projection projection;
};

struct World {
    Window window;
    Camera camera;
    std::vector<std::string> models;
};

#endif