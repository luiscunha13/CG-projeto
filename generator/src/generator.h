#ifndef GENERATOR_H
#define GENERATOR_H

#include"../../common/include/utils.h"

#include <vector>
#include <math.h>

class generator {
    std::vector<Point> generatePlane(float length, int divisions);
    std::vector<Point> generateBox(float length, int divisions);
    std::vector<Point> generateSphere(float radius, int slices, int stacks);
    std::vector<Point> generateCone(float radius, float height, int slices, int stacks);
};


#endif
