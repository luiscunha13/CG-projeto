#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <vertex.h>
#include <Light.h>
#include <GL/gl.h>
#include <string>

struct Model {
    int n_vertices;
    int n_indices;
    std::vector<Vertex3f> vertices;
    std::vector<unsigned int> indices;
    std::vector<Transformation> transformations;
    //adicionado
    std::vector<Vertex3f> normals;
    std::vector<Vertex2f> texCoords;
    Texture texture;
    bool hasTexture;
    Material material;
    bool hasMaterial;

    GLuint vertexBuffer;
    GLuint indexBuffer;
    bool vboInitialized;
    //adicionado
    GLuint normalBuffer;
    GLuint texCoordBuffer;
};

#endif