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
    cout << "Vertices" << endl;
    for (int z = 0; z < divisions + 1; ++z) {
        for (int x = 0; x < divisions + 1; ++x) {
            cout << -middle + x * side << " 0 " << -middle + z * side << endl;
            vertices.push_back({-middle + x * side, 0, -middle + z * side});
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

    vector<Vertex3f> vertices;
    vector<unsigned int> indexes;
    vector<Vertex3f> normals;
    vector<Vertex2f> texCoords;

    cout << "Vertices" << endl;
    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i <= divisions; ++i) {
            for (int j = 0; j <= divisions; ++j) {
                float v = -halfLength + i * step;
                float u = -halfLength + j * step;

                Vertex3f vertex;
                Vertex3f normal;
                Vertex2f texCoord;
                switch (face) {
                    case 0: // frente
                        vertex = {v, u, halfLength};
                        normal = {0.0f, 0.0f, 1.0f};
                        texCoord = {(j * step)/length, (i * step)/length};
                        break;
                    case 1: // trás
                        vertex = {v, u, -halfLength};
                        normal = {0.0f, 0.0f, -1.0f};
                        texCoord = {1.0f - (j * step)/length, (i * step)/length};
                        break;
                    case 2: // esquerda
                        vertex = {-halfLength, v, u};
                        normal = {-1.0f, 0.0f, 0.0f};
                        texCoord = {(j * step)/length, (i * step)/length};
                        break;
                    case 3: // direita
                        vertex = {halfLength, v, u};
                        normal = {1.0f, 0.0f, 0.0f};
                        texCoord = {1.0f - (j * step)/length, (i * step)/length};
                        break;
                    case 4: // cima
                        vertex = {v, halfLength, u};
                        normal = {0.0f, 1.0f, 0.0f};
                        texCoord = {(j * step)/length, 1.0f - (i * step)/length};
                        break;
                    case 5: // baixo
                        vertex = {v, -halfLength, u};
                        normal = {0.0f, -1.0f, 0.0f};
                        texCoord = {(j * step)/length, (i * step)/length};
                        break;
                }
                cout << vertex.x << vertex.y  << vertex.z << endl;
                vertices.push_back(vertex);
                normals.push_back(normal);
                texCoords.push_back(texCoord);
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

    return {vertices, indexes, normals, texCoords};
}


Generator generateSphere(float radius, int stacks, int slices) {
    vector<Vertex3f> vertices;
    vector<unsigned int> indexes;

    cout << "Vertices" << endl;
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = i * (M_PI / stacks) - M_PI / 2 ; //pi/2 a -pi/2
        float xy = radius * cos(stackAngle); // distancia horizontal ao eixo
        float z = radius * sin(stackAngle); // altura

        for (int j = 0; j <= slices; ++j) {
            float sliceAngle = j * (2 * M_PI / slices); // 0 a 2pi
            float y = xy * cos(sliceAngle);
            float x = xy * sin(sliceAngle);

            cout << x << y  << z << endl;
            vertices.push_back({x, y, z});
        }
    }

    cout << "Indices" << endl;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            const uint32_t first = i * (slices + 1) + j;
            const uint32_t second = first + slices + 1;

            cout << first << " " << second << " " << second + 1 << endl;
            cout << first << " " << second + 1 << " " << first + 1 << endl;
            add3Items(first, second, second + 1, indexes);
            add3Items(first, second + 1, first + 1, indexes);
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
    const Vertex3f base_center = {0, 0, 0};
    cout << 0 << 0  << 0 << endl;
    vertices.push_back(base_center);

    for (int slice = 0; slice <= slices; ++slice) {
        float angle = slice * slice_size;

        for (int stack = 0; stack <= stacks; ++stack) {
            const float current_radius = radius - stack * radius / stacks;
            float current_x = current_radius * cos(angle);
            float current_z = current_radius * sin(angle);
            float current_y = stack * stack_size;

            cout << current_x << current_y  << current_z << endl;
            vertices.push_back({current_x, current_y, current_z});
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
    const Vertex3f base_center = {0, 0, 0};
    const Vertex3f top_center = {0, height, 0};
    cout << 0 << 0 << 0 << endl;
    cout << 0 << height << 0 << endl;
    vertices.push_back(base_center);
    vertices.push_back(top_center);


    for (int slice = 0; slice <= slices; ++slice) {
        float angle = slice * slice_size;
        float cosA = cos(angle);
        float sinA = sin(angle);

        cout << radius * cosA << 0 << radius * sinA << endl;
        vertices.push_back({radius * cosA, 0, radius * sinA});
        cout << radius * cosA << height << radius * sinA << endl;
        vertices.push_back({radius * cosA, height, radius * sinA});
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

        for (unsigned int j = 0; j <= stacks; ++j) {
            float stackAngle = j * stack_size;
            float cosStack = cos(stackAngle);
            float sinStack = sin(stackAngle);

            float x = (outerRadius + innerRadius * cosStack) * cosSlice;
            float y = (outerRadius + innerRadius * cosStack) * sinSlice;
            float z = innerRadius * sinStack;

            cout << x << y << z << endl;
            vertices.push_back({x, y, z});
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

            float Bu[4] = {
                    u1 * u1 * u1,
                    binom[1] * u1 * u1 * u,
                    binom[2] * u1 * u * u,
                    u * u * u
            };

            for (int j = 0; j <= tessellation; ++j) {
                const float v = float(j) / tessellation;
                const float v1 = 1.0f - v;

                float Bv[4] = {
                        v1 * v1 * v1,
                        binom[1] * v1 * v1 * v,
                        binom[2] * v1 * v * v,
                        v * v * v
                };

                Vertex3f point = {0, 0, 0};
                for (int k = 0; k < 4; ++k) {
                    for (int l = 0; l < 4; ++l) {
                        // Get the control point using the indices from the patch
                        const Vertex3f& cp = controlPoints[indices[k * 4 + l]];
                        point.x += cp.x * Bu[k] * Bv[l];
                        point.y += cp.y * Bu[k] * Bv[l];
                        point.z += cp.z * Bu[k] * Bv[l];
                    }
                }

                cout << point.x << " " << point.y << " " << point.z << endl;
                vertices.push_back(point);
            }
        }

        cout << "Indices" << endl;
        for (int i = 0; i < tessellation; ++i) {
            for (int j = 0; j < tessellation; ++j) {
                const uint32_t topLeft = baseIndex + i * (tessellation + 1) + j;
                const uint32_t topRight = topLeft + 1;
                const uint32_t bottomLeft = topLeft + (tessellation + 1);
                const uint32_t bottomRight = bottomLeft + 1;

                cout << topLeft << " " << bottomLeft << " " << bottomRight << endl;
                cout << topLeft << " " << bottomRight << " " << topRight << endl;
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

        // Escrever vértices
        //isto no caso dos 3 terem o mesmo tamanho, se não é preciso separa em 3 ciclos
        for (size_t i = 0; i < result.vertices.size(); i++) {
          	file << result.vertices[i].x << " " << result.vertices[i].y << " " << result.vertices[i].z << std::endl;
            //adicionado
          	file << result.normals[i].x << " " << result.normals[i].y << " " << result.normals[i].z << std::endl;
        	file << result.texCoords[i].x << " " << result.texCoords[i].y << std::endl;
          }
        /*
        for (const auto &vertex : result.vertices)
        {
            file << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
        }
        */




        // Escrever faces (triângulos)
        for (size_t i = 0; i < result.indices.size(); i += 3)
        {
            file << result.indices[i] << " "
                 << result.indices[i+1] << " "
                 << result.indices[i+2] << std::endl;
        }

        file.close();
        return true;
    }

}
