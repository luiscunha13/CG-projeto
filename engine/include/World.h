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

// Structure to hold model data
struct Model {
    std::string file;
};

struct World {
    Window window;
    Camera camera;
    std::vector<Model> models;
};


#endif