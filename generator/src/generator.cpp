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
    cout << "VErtices" << endl;
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

Generator GenerateBox(float length, size_t divisions)
    {
        Generator result;
        float halfLength = length / 2.0f;
        float step = length / divisions;

        // Array de direções normais para cada face da caixa
        Vertex3f faceNormals[6] = {
            Vertex3f(0.0f, 0.0f, 1.0f),   // Frente (Z+)
            Vertex3f(0.0f, 0.0f, -1.0f),  // Trás (Z-)
            Vertex3f(0.0f, 1.0f, 0.0f),   // Topo (Y+)
            Vertex3f(0.0f, -1.0f, 0.0f),  // Base (Y-)
            Vertex3f(1.0f, 0.0f, 0.0f),   // Direita (X+)
            Vertex3f(-1.0f, 0.0f, 0.0f)   // Esquerda (X-)
        };

        // Gerar vértices para cada face
        for (size_t face = 0; face < 6; face++)
        {
            for (size_t i = 0; i <= divisions; i++)
            {
                for (size_t j = 0; j <= divisions; j++)
                {
                    Vertex3f pos;

                    // Calcular posição do vértice com base na face
                    switch (face)
                    {
                        case 0: // Frente (Z+)
                            pos = Vertex3f(-halfLength + j * step, -halfLength + i * step, halfLength);
                            break;
                        case 1: // Trás (Z-)
                            pos = Vertex3f(halfLength - j * step, -halfLength + i * step, -halfLength);
                            break;
                        case 2: // Topo (Y+)
                            pos = Vertex3f(-halfLength + j * step, halfLength, halfLength - i * step);
                            break;
                        case 3: // Base (Y-)
                            pos = Vertex3f(-halfLength + j * step, -halfLength, -halfLength + i * step);
                            break;
                        case 4: // Direita (X+)
                            pos = Vertex3f(halfLength, -halfLength + i * step, halfLength - j * step);
                            break;
                        case 5: // Esquerda (X-)
                            pos = Vertex3f(-halfLength, -halfLength + i * step, -halfLength + j * step);
                            break;
                    }

                    // Adicionar vértice
                    result.vertices.push_back(pos);
                }
            }
        }

        // Gerar índices para os triângulos
        size_t verticesPerFace = (divisions + 1) * (divisions + 1);

        for (size_t face = 0; face < 6; face++)
        {
            size_t faceOffset = face * verticesPerFace;

            for (size_t i = 0; i < divisions; i++)
            {
                for (size_t j = 0; j < divisions; j++)
                {
                    uint32_t p0 = faceOffset + i * (divisions + 1) + j;
                    uint32_t p1 = p0 + 1;
                    uint32_t p2 = p0 + (divisions + 1);
                    uint32_t p3 = p2 + 1;

                    // Primeiro triângulo
                    result.indices.push_back(p0);
                    result.indices.push_back(p2);
                    result.indices.push_back(p1);

                    // Segundo triângulo
                    result.indices.push_back(p1);
                    result.indices.push_back(p2);
                    result.indices.push_back(p3);
                }
            }
        }

        return result;
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
