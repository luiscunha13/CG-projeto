#ifndef GENERATOR_H
#define GENERATOR_H

#include"../../common/include/utils.h"

#include <vector>
#include <math.h>

namespace generator{

    struct Generator{
      std::vector<Vec3f> vertices;
      std::vector<uint32_t> indices;
    };

    Generator generatePlane(float length, int divisions)
    Generator generateBox(float length, int divisions)
    Generator generateSphere(float radius, int slices, int stacks)
    Generator generateCone(float radius, float height, int slices, int stacks)

    bool SaveModel(const Generator& generator, const std::string& filename);

}

#endif
