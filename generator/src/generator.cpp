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

Generator generateBox(float length, int divisions) {
    float step = length / divisions;
    float halfLength = length / 2.0f;

    vector<Vertex3f> vertices;
    vector<unsigned int> indexes;

    cout << "Vertices" << endl;
    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i <= divisions; ++i) {
            for (int j = 0; j <= divisions; ++j) {
                float v = -halfLength + i * step;
                float u = -halfLength + j * step;

                Vertex3f vertex;
                switch (face) {
                    case 0: // frente
                        vertex = {v, u, halfLength};
                        break;
                    case 1: // trás
                        vertex = {v, u, -halfLength};
                        break;
                    case 2: // esquerda
                        vertex = {-halfLength, v, u};
                        break;
                    case 3: // direita
                        vertex = {halfLength, v, u};
                        break;
                    case 4: // cima
                        vertex = {v, halfLength, u};
                        break;
                    case 5: // baixo
                        vertex = {v, -halfLength, u};
                        break;
                }
                cout << vertex.x << vertex.y  << vertex.z << endl;
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

                cout << top_left << " " << bottom_left << " " << bottom_right << endl;
                cout << top_left << " " << bottom_right << " " << top_right << endl;
                add3Items(top_left, bottom_left, bottom_right, indexes);
                add3Items(top_left, bottom_right, top_right, indexes);
            }
        }
    }

    return {vertices, indexes};
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

    const Vertex3f base_center = {0, 0, 0};
    vertices.push_back(base_center);

    for (int slice = 0; slice <= slices; ++slice) {
        float angle = slice * slice_size;

        for (int stack = 0; stack <= stacks; ++stack) {
            const float current_radius = radius - stack * radius / stacks;
            float current_x = current_radius * cos(angle);
            float current_z = current_radius * sin(angle);
            float current_y = stack * stack_size;

            vertices.push_back({current_x, current_y, current_z});
        }
    }

    for (int slice = 0; slice < slices; ++slice) {
        for (int stack = 0; stack < stacks; ++stack) {
            uint32_t bottom_left_index = 1 + stack + (slice * (stacks + 1));
            uint32_t bottom_right_index = bottom_left_index + stacks + 1;

            uint32_t top_left_index = bottom_left_index + 1;
            uint32_t top_right_index = bottom_right_index + 1;

            add3Items(top_left_index, bottom_left_index, bottom_right_index, indexes);

            if (stack != stacks - 1)
                add3Items(top_left_index, bottom_right_index, top_right_index, indexes);
        }

        uint32_t base_left_index = 1 + (slice * (stacks + 1));
        uint32_t base_right_index = base_left_index + stacks + 1;

        indexes.insert(indexes.end(), {base_right_index, base_left_index, 0}); // base triangle
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
        for (const auto &vertex : result.vertices)
        {
            file << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
        }

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
