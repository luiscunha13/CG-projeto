#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <vertex.h>
#include <Light.h>
#include <GL/gl.h>
#include <string>

struct BoundingSphere {
    Vertex3f center;
    float radius;
};

struct Model {
    int n_vertices;
    int n_indices;
    std::vector<Vertex3f> vertices;
    std::vector<unsigned int> indices;
    std::vector<Transformation> transformations;

    GLuint textureID;
    bool hasTexture;
    Material material;

    GLuint vertexBuffer;
    GLuint indexBuffer;
    bool vboInitialized;

    BoundingSphere boundingSphere;
    bool hasBoundingSphere;

    bool isSkybox;
};

#endif