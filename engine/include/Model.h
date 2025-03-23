#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <vertex.h>

struct Model {
    int n_vertices;
    int n_indices;
    std::vector<Vertex3f> vertices;
    std::vector<unsigned int> indices;
    std::vector<Transformation> transformations;
};

#endif