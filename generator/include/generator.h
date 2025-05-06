#ifndef GENERATOR_H
#define GENERATOR_H

#include"../../common/include/vertex.h"
#include <string>
#include <cstdint>
#include <vector>
#include <math.h>

namespace generator{

    struct Generator{
      std::vector<Vertex3f> vertices;
      std::vector<uint32_t> indices;
       //adicionado
      std::vector<Vertex3f> normals;
      std::vector<Vertex2f> texCoords;
    };

    Generator generatePlane(float size, int divisions);
    Generator generateBox(float length, int divisions);
    Generator generateSphere(float radius, int slices, int stacks);
    Generator generateCone(float radius, float height, unsigned int slices, unsigned int stacks);
    Generator generateCylinder(float radius, float height, unsigned int slices);
    Generator generateTorus(float outerRadius, float innerRadius, unsigned int slices, unsigned int stacks);
    Generator generateBezier(const std::string& patchFile, int tessellation);

    bool SaveModel(const Generator& generator, const std::string& filename);

}

#endif
