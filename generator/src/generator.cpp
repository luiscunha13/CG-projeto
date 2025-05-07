#include <generator.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

using namespace std;

namespace generator {

template <typename T>
void add3Items(const T& i1, const T& i2, const T& i3, vector<T> &items) {
    items.push_back(i1);
    items.push_back(i2);
    items.push_back(i3);
}

Generator generatePlane(float size, int divisions) {
    vector<Vertex3f> vertices;
    vector<unsigned int> indexes;

    const float side = size / divisions;
    const float middle = size / 2;
    const float texStep = 1.0f / divisions;
    cout << "Vertices" << endl;
    for (int z = 0; z < divisions + 1; ++z) {
        for (int x = 0; x < divisions + 1; ++x) {
            Vertex3f vertex;
            vertex.x = -middle + x * side;
            vertex.y = 0;
            vertex.z = -middle + z * side;
            vertex.nx = 0;  // Normal points up (Y-axis)
            vertex.ny = 1;
            vertex.nz = 0;
            vertex.s = x * texStep;  // Texture coordinates
            vertex.t = z * texStep;
            vertices.push_back(vertex);
            cout << -middle + x * side << " 0 " << -middle + z * side << endl;
            vertices.push_back(vertex);
        }
    }

    cout << "Indices" << endl;
    for (int z = 0; z < divisions; ++z) {
        for (int x = 0; x < divisions; ++x) {
            const uint32_t top_left = x * (divisions + 1) + z;
            const uint32_t top_right = top_left + 1;
            const uint32_t bottom_left = (x + 1) * (divisions + 1) + z;
            const uint32_t bottom_right = bottom_left + 1;
            cout << top_left << " " << bottom_left << " " << bottom_right << endl;
            cout << top_left << " " << bottom_right << " " << top_right << endl;
            add3Items(top_left, bottom_left, bottom_right, indexes);
            add3Items(top_left, bottom_right, top_right, indexes);
        }
    }

    return {vertices, indexes};
}
    //adicionado normais e texCoords
Generator generateBox(float length, int divisions) {
    float step = length / divisions;
    float halfLength = length / 2.0f;
    float texStep = 1.0f / divisions;

    vector<Vertex3f> vertices;
    vector<unsigned int> indexes;

    cout << "Vertices" << endl;
    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i <= divisions; ++i) {
            for (int j = 0; j <= divisions; ++j) {
                float v = -halfLength + i * step;
                float u = -halfLength + j * step;
                float texU = j * texStep;
                float texV = i * texStep;

                Vertex3f vertex;
                switch (face) {
                    case 0: // front
                        vertex = {v, u, halfLength, 0, 0, 1, texU, texV};
                        break;
                    case 1: // back
                        vertex = {v, u, -halfLength, 0, 0, -1, texU, texV};
                        break;
                    case 2: // left
                        vertex = {-halfLength, v, u, -1, 0, 0, texU, texV};
                        break;
                    case 3: // right
                        vertex = {halfLength, v, u, 1, 0, 0, texU, texV};
                        break;
                    case 4: // top
                        vertex = {v, halfLength, u, 0, 1, 0, texU, texV};
                        break;
                    case 5: // bottom
                        vertex = {v, -halfLength, u, 0, -1, 0, texU, texV};
                        break;
                }
                vertices.push_back(vertex);
            }
        }
    }

    cout << "Indices" << endl;
    for (int face = 0; face < 6; ++face) {
        int offset = face * (divisions + 1) * (divisions + 1);
        for (int i = 0; i < divisions; ++i) {
            for (int j = 0; j < divisions; ++j) {
                const uint32_t top_left = offset + i * (divisions + 1) + j;
                const uint32_t top_right = top_left + 1;
                const uint32_t bottom_left = offset + (i + 1) * (divisions + 1) + j;
                const uint32_t bottom_right = bottom_left + 1;
                
                if (face == 1 || face == 2 || face == 4) {
                    add3Items(top_left, bottom_right, bottom_left, indexes);
                    add3Items(top_left, top_right, bottom_right, indexes);
                } else {
                    add3Items(top_left, bottom_left, bottom_right, indexes);
                    add3Items(top_left, bottom_right, top_right, indexes);
                }
            }
        }
    }

    return {vertices, indexes};
}


    Generator generateSphere(float radius, int stacks, int slices) {
        vector<Vertex3f> vertices;
        vector<unsigned int> indexes;

        // Generate vertices
        for (int i = 0; i <= stacks; ++i) {
            float stackAngle = i * (M_PI / stacks) - M_PI / 2;  // -pi/2 to pi/2
            float xy = radius * cos(stackAngle);  // horizontal distance to axis
            float z = radius * sin(stackAngle);   // height

            // Correct texture V coordinate (0 to 1 from bottom to top)
            float texV = 1.0f - (float)i / stacks;

            for (int j = 0; j <= slices; ++j) {
                float sliceAngle = j * (2 * M_PI / slices);  // 0 to 2pi
                float x = xy * sin(sliceAngle);  // Use sin for x
                float y = xy * cos(sliceAngle);  // Use cos for y

                // Correct texture U coordinate (0 to 1 around the sphere)
                float texU = (float)j / slices;

                // Proper normal calculation (unit vector from center)
                float nx = x / radius;
                float ny = y / radius;
                float nz = z / radius;

                Vertex3f vertex = {x, y, z, nx, ny, nz, texU, texV};
                vertices.push_back(vertex);
            }
        }

        // Generate indices
        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < slices; ++j) {
                const uint32_t first = i * (slices + 1) + j;
                const uint32_t second = first + slices + 1;

                // First triangle
                indexes.push_back(first);
                indexes.push_back(second);
                indexes.push_back(second + 1);

                // Second triangle
                indexes.push_back(first);
                indexes.push_back(second + 1);
                indexes.push_back(first + 1);
            }
        }

        return {vertices, indexes};
    }

Generator generateCone(float radius, float height, unsigned int slices, unsigned int stacks) {
    vector<Vertex3f> vertices;
    vector<uint32_t> indexes;

    const float stack_size = height / stacks;
    const double slice_size = 2 * M_PI / slices;

    cout << "Vertices" << endl;
    Vertex3f base_center = {0, 0, 0, 0, -1, 0, 0.5f, 0.5f};
    cout << 0 << 0  << 0 << endl;
    vertices.push_back(base_center);

    for (int slice = 0; slice <= slices; ++slice) {
        float angle = slice * slice_size;
        float texU = (float)slice / slices;

        for (int stack = 0; stack <= stacks; ++stack) {
            const float current_radius = radius - stack * radius / stacks;
            float current_x = current_radius * cos(angle);
            float current_z = current_radius * sin(angle);
            float current_y = stack * stack_size;
            float texV = (float)stack / stacks;

            float normal_x = cos(angle);
            float normal_y = radius / height;
            float normal_z = sin(angle);
            // Normalizar
            float length = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z);
            normal_x /= length;
            normal_y /= length;
            normal_z /= length;

            cout << current_x << current_y  << current_z << endl;
            Vertex3f vertex = {
                    current_x, current_y, current_z,
                    normal_x, normal_y, normal_z,
                    texU, texV
            };
            vertices.push_back(vertex);
        }
    }

    cout << "Indices" << endl;
    for (int slice = 0; slice < slices; ++slice) {
        for (int stack = 0; stack < stacks; ++stack) {
            uint32_t bottom_left_index = 1 + stack + (slice * (stacks + 1));
            uint32_t bottom_right_index = bottom_left_index + stacks + 1;

            uint32_t top_left_index = bottom_left_index + 1;
            uint32_t top_right_index = bottom_right_index + 1;

            cout << top_left_index << " " << bottom_left_index<< " " << bottom_right_index << endl;
            add3Items(top_left_index, bottom_left_index, bottom_right_index, indexes);

            if (stack != stacks - 1){
                cout << top_left_index << " " << bottom_right_index<< " " << top_right_index << endl;
                add3Items(top_left_index, bottom_right_index, top_right_index, indexes);
            }

        }

        uint32_t base_left_index = 1 + (slice * (stacks + 1));
        uint32_t base_right_index = base_left_index + stacks + 1;

        cout << base_right_index << " " << base_left_index<< " " << 0 << endl;
        add3Items(base_right_index, base_left_index, (uint32_t)0, indexes); // base triangle
    }

    return {vertices, indexes};
}

Generator generateCylinder(float radius, float height, unsigned int slices) {
    vector<Vertex3f> vertices;
    vector<uint32_t> indexes;

    const double slice_size = 2 * M_PI / slices;

    cout << "Vertices" << endl;
    Vertex3f base_center = {0, 0, 0, 0, -1, 0, 0.5f, 0.5f};
    Vertex3f top_center = {0, height, 0, 0, 1, 0, 0.5f, 0.5f};
    cout << 0 << 0 << 0 << endl;
    cout << 0 << height << 0 << endl;
    vertices.push_back(base_center);
    vertices.push_back(top_center);


    for (int slice = 0; slice <= slices; ++slice) {
        float angle = slice * slice_size;
        float cosA = cos(angle);
        float sinA = sin(angle);

        cout << radius * cosA << 0 << radius * sinA << endl;
        Vertex3f base_vertex = {
                radius * cosA, 0, radius * sinA,
                0, -1, 0,
                (cosA + 1) * 0.5f, (sinA + 1) * 0.5f
        };
        vertices.push_back(base_vertex);
        cout << radius * cosA << height << radius * sinA << endl;
        Vertex3f top_vertex = {
                radius * cosA, height, radius * sinA,
                0, 1, 0,
                (cosA + 1) * 0.5f, (sinA + 1) * 0.5f
        };
        vertices.push_back(top_vertex);
        /*
        Vertex3f side_base = {
                radius * cosA, 0, radius * sinA,
                cosA, 0, sinA,
                texU, 0
        };
        Vertex3f side_top = {
                radius * cosA, height, radius * sinA,
                cosA, 0, sinA,
                texU, 1
        };
        vertices.push_back(side_base);
        vertices.push_back(side_top);
         */
    }

    cout << "Indices" << endl;
    for (int slice = 0; slice < slices; ++slice) {
        uint32_t bottom_left_index = 2 + (slice * 2);
        uint32_t bottom_right_index = 2 + ((slice + 1) * 2);

        uint32_t top_left_index = bottom_left_index + 1;
        uint32_t top_right_index = bottom_right_index + 1;


        // First triangle (bottom left, top left, bottom right)
        cout << bottom_left_index << " " << top_left_index<< " " << bottom_right_index << endl;
        add3Items(bottom_left_index, top_left_index, bottom_right_index, indexes);

        // Second triangle (top left, top right, bottom right)
        cout << top_left_index << " " << top_right_index<< " " << bottom_right_index << endl;
        add3Items(top_left_index, top_right_index, bottom_right_index, indexes);

        // Base triangle (base center, base left, base right)
        cout << 0 << " " << bottom_left_index<< " " << bottom_right_index << endl;
        add3Items((uint32_t)0, bottom_left_index, bottom_right_index, indexes);

        // Top triangle (top center, top right, top left)
        cout << 1 << " " << top_right_index<< " " << top_left_index << endl;
        add3Items((uint32_t)1, top_right_index, top_left_index, indexes);
    }


    return {vertices, indexes};
}

Generator generateTorus(float outerRadius, float innerRadius, unsigned int slices, unsigned int stacks) {
    vector<Vertex3f> vertices;
    vector<uint32_t> indexes;

    const float slice_size = 2 * M_PI / slices;
    const float stack_size = 2 * M_PI / stacks;

    cout << "Vertices" << endl;
    for (unsigned int i = 0; i <= slices; ++i) {
        float sliceAngle = i * slice_size;
        float cosSlice = cos(sliceAngle);
        float sinSlice = sin(sliceAngle);
        float texU = (float)i / slices;

        for (unsigned int j = 0; j <= stacks; ++j) {
            float stackAngle = j * stack_size;
            float cosStack = cos(stackAngle);
            float sinStack = sin(stackAngle);
            float texV = (float)j / stacks;

            float x = (outerRadius + innerRadius * cosStack) * cosSlice;
            float y = (outerRadius + innerRadius * cosStack) * sinSlice;
            float z = innerRadius * sinStack;

            float nx = cosSlice * cosStack;
            float ny = sinSlice * cosStack;
            float nz = sinStack;

            cout << x << y << z << endl;
            Vertex3f vertex = {x, y, z, nx, ny, nz, texU, texV};
            vertices.push_back(vertex);
        }
    }

    cout << "Indices" << endl;
    for (unsigned int i = 0; i < slices; ++i) {
        for (unsigned int j = 0; j < stacks; ++j) {
            uint32_t current = i * (stacks + 1) + j;
            uint32_t next = current + stacks + 1;

            cout << current << " " << next << " " << current+1 << endl;
            cout << next << " " << next+1 << " " << current+1 << endl;
            add3Items(current, next, current + 1,indexes);
            add3Items(next, next + 1, current + 1,indexes);
        }
    }

    return {vertices, indexes};
}

Generator generateBezier(const std::string& patchFile, int tessellation) {
    vector<Vertex3f> vertices;
    vector<unsigned int> indexes;
    vector<Vertex3f> controlPoints;

    ifstream file(patchFile);
    if (!file.is_open()) {
        cerr << "Error opening patch file: " << patchFile << endl;
        return {vertices, indexes};
    }

    int numPatches;
    file >> numPatches;

    vector<vector<int>> patchIndices(numPatches, vector<int>(16));
    for (int p = 0; p < numPatches; ++p) {
        for (int i = 0; i < 16; ++i) {
            file >> patchIndices[p][i];
            file.ignore();
        }
    }

    int numControlPoints;
    file >> numControlPoints;

    controlPoints.resize(numControlPoints);
    for (int i = 0; i < numControlPoints; ++i) {
        file >> controlPoints[i].x;
        file.ignore();
        file >> controlPoints[i].y;
        file.ignore();
        file >> controlPoints[i].z;
        file.ignore();

    }

    const float binom[4] = {1.0f, 3.0f, 3.0f, 1.0f};

    cout << "Vertices" << endl;
    for (const auto& indices : patchIndices) {
        const int baseIndex = vertices.size();

        // Generate vertices
        for (int i = 0; i <= tessellation; ++i) {
            const float u = float(i) / tessellation;
            const float u1 = 1.0f - u;
            float texU = u;

            float Bu[4] = {
                    u1 * u1 * u1,
                    binom[1] * u1 * u1 * u,
                    binom[2] * u1 * u * u,
                    u * u * u
            };

            float dBu[4] = {
                    -3 * u1 * u1,
                    3 * u1 * u1 - 6 * u1 * u,
                    6 * u1 * u - 3 * u * u,
                    3 * u * u
            };

            for (int j = 0; j <= tessellation; ++j) {
                const float v = float(j) / tessellation;
                const float v1 = 1.0f - v;
                float texV = v;

                float Bv[4] = {
                        v1 * v1 * v1,
                        binom[1] * v1 * v1 * v,
                        binom[2] * v1 * v * v,
                        v * v * v
                };

                float dBv[4] = {
                        -3 * v1 * v1,
                        3 * v1 * v1 - 6 * v1 * v,
                        6 * v1 * v - 3 * v * v,
                        3 * v * v
                };

                Vertex3f point = {0, 0, 0, 0, 0, 0, texU, texV};
                Vertex3f tangentU = {0, 0, 0, 0, 0, 0, 0, 0};
                Vertex3f tangentV = {0, 0, 0, 0, 0, 0, 0, 0};

                for (int k = 0; k < 4; ++k) {
                    for (int l = 0; l < 4; ++l) {
                        const Vertex3f& cp = controlPoints[indices[k * 4 + l]];

                        // Position
                        point.x += cp.x * Bu[k] * Bv[l];
                        point.y += cp.y * Bu[k] * Bv[l];
                        point.z += cp.z * Bu[k] * Bv[l];

                        // Tangent in u direction
                        tangentU.x += cp.x * dBu[k] * Bv[l];
                        tangentU.y += cp.y * dBu[k] * Bv[l];
                        tangentU.z += cp.z * dBu[k] * Bv[l];

                        // Tangent in v direction
                        tangentV.x += cp.x * Bu[k] * dBv[l];
                        tangentV.y += cp.y * Bu[k] * dBv[l];
                        tangentV.z += cp.z * Bu[k] * dBv[l];
                    }
                }

                // Calculate normal as cross product of tangents
                point.nx = tangentU.y * tangentV.z - tangentU.z * tangentV.y;
                point.ny = tangentU.z * tangentV.x - tangentU.x * tangentV.z;
                point.nz = tangentU.x * tangentV.y - tangentU.y * tangentV.x;

                // Normalize the normal
                float len = sqrt(point.nx*point.nx + point.ny*point.ny + point.nz*point.nz);
                if (len > 0) {
                    point.nx /= len;
                    point.ny /= len;
                    point.nz /= len;
                }

                cout << point.x << " " << point.y << " " << point.z << endl;
                vertices.push_back(point);
            }
        }

        // Index generation remains the same as original
        for (int i = 0; i < tessellation; ++i) {
            for (int j = 0; j < tessellation; ++j) {
                const uint32_t topLeft = baseIndex + i * (tessellation + 1) + j;
                const uint32_t topRight = topLeft + 1;
                const uint32_t bottomLeft = topLeft + (tessellation + 1);
                const uint32_t bottomRight = bottomLeft + 1;

                add3Items(topLeft, bottomLeft, bottomRight, indexes);
                add3Items(topLeft, bottomRight, topRight, indexes);
            }
        }
    }

    return {vertices, indexes};
}

bool SaveModel(const Generator &result, const std::string &filename)
    {
        ofstream file(filename);
        if (!file)
        {
            cerr << "Erro ao abrir arquivo: " << filename << std::endl;
            return false;
        }

        // Escrever cabeçalho
        file << result.vertices.size() << " " << result.indices.size() / 3 << std::endl;


        for (const auto &vertex : result.vertices) {
            file << vertex.x << " " << vertex.y << " " << vertex.z << " "
                 << vertex.nx << " " << vertex.ny << " " << vertex.nz << " "
                 << vertex.s << " " << vertex.t << std::endl;
        }

        // Write faces (triangles)
        for (size_t i = 0; i < result.indices.size(); i += 3) {
            file << result.indices[i] << " "
                 << result.indices[i+1] << " "
                 << result.indices[i+2] << std::endl;
        }

        file.close();
        return true;
    }

}
