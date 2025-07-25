#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <vector>
#include <vertex.h>
#include <Light.h>


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

struct Transformation{
    enum class Type {
        Translate,
        Rotate,
        Scale
    };

    Type type;
    Vertex3f coords;
    float angle;

    bool animated;

    struct {
        float time;
        bool align;
        std::vector<Vertex3f> points;
        std::string algorithm;
    } animation;

};

struct ModelInfo {
    std::string file;
    std::string texture;
    Material material;
    bool isSkybox;
};

struct Group {
    std::vector<Transformation> transformations;
    std::vector<ModelInfo> models;
    std::vector<Group> childGroups;
};


struct World {
    Window window;
    Camera camera;
    std::vector<Light> lights;
    std::vector<Group> groups;
};

#endif