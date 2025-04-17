#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <vertex.h>
#include <GL/gl.h>

struct Model {
    int n_vertices;
    int n_indices;
    std::vector<Vertex3f> vertices;
    std::vector<unsigned int> indices;
    std::vector<Transformation> transformations;

    GLuint vertexBuffer;
    GLuint indexBuffer;
    bool vboInitialized;
};

#endif