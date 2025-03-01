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

    const auto side = size / divisions;
    const auto middle = size / 2;
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


Generator GenerateSphere(float radius, int slices, int stacks){

}

Generator GenerateCone(float radius, float height, int slices, int stacks){

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
