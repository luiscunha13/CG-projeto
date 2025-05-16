#ifndef LIGHT_H
#define LIGHT_H

#include <vertex.h>
#include <string>

struct Material {
    Vertex3f diffuse;
    Vertex3f ambient;
    Vertex3f specular;
    Vertex3f emissive;
    float shininess;
};


struct Light {
    enum Type { POINT, DIRECTIONAL, SPOT };
    Type type;
    Vertex3f position;
    Vertex3f direction;
    float cutoff;

};

#endif // LIGHT_H
