#include "generator.h"
#include"../../common/include/utils.h"

#include <vector>
#include <math.h>


namespace generator {


std::vector<Point> generatePlane(float length, int divisions){

}

Generator GenerateBox(float length, size_t divisions)
    {
        Generator result;
        float halfLength = length / 2.0f;
        float step = length / divisions;

        // Array de direções normais para cada face da caixa
        Vec3f faceNormals[6] = {
            Vec3f(0.0f, 0.0f, 1.0f),   // Frente (Z+)
            Vec3f(0.0f, 0.0f, -1.0f),  // Trás (Z-)
            Vec3f(0.0f, 1.0f, 0.0f),   // Topo (Y+)
            Vec3f(0.0f, -1.0f, 0.0f),  // Base (Y-)
            Vec3f(1.0f, 0.0f, 0.0f),   // Direita (X+)
            Vec3f(-1.0f, 0.0f, 0.0f)   // Esquerda (X-)
        };

        // Função para calcular coordenadas UV por face
        auto calculateUV = [&](size_t faceIndex, size_t i, size_t j) -> Vec2f {
            float u = static_cast<float>(j) / divisions;
            float v = static_cast<float>(i) / divisions;
            return Vec2f(u, v);
        };

        // Gerar vértices para cada face
        for (size_t face = 0; face < 6; face++)
        {
            for (size_t i = 0; i <= divisions; i++)
            {
                for (size_t j = 0; j <= divisions; j++)
                {
                    Vec3f pos;

                    // Calcular posição do vértice com base na face
                    switch (face)
                    {
                        case 0: // Frente (Z+)
                            pos = Vec3f(-halfLength + j * step, -halfLength + i * step, halfLength);
                            break;
                        case 1: // Trás (Z-)
                            pos = Vec3f(halfLength - j * step, -halfLength + i * step, -halfLength);
                            break;
                        case 2: // Topo (Y+)
                            pos = Vec3f(-halfLength + j * step, halfLength, halfLength - i * step);
                            break;
                        case 3: // Base (Y-)
                            pos = Vec3f(-halfLength + j * step, -halfLength, -halfLength + i * step);
                            break;
                        case 4: // Direita (X+)
                            pos = Vec3f(halfLength, -halfLength + i * step, halfLength - j * step);
                            break;
                        case 5: // Esquerda (X-)
                            pos = Vec3f(-halfLength, -halfLength + i * step, -halfLength + j * step);
                            break;
                    }

                    // Adicionar vértice
                    result.vertices.push_back(pos);

                    // Adicionar normal
                    result.normals.push_back(faceNormals[face]);

                    // Adicionar coordenada de textura
                    result.tex_coords.push_back(calculateUV(face, i, j));
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
}

std::vector<Point> generateSphere(float radius, int slices, int stacks){

}

std::vector<Point> generateCone(float radius, float height, int slices, int stacks){

}

bool SaveModel(const GeneratorResult &result, const std::string &filename)
    {
        std::ofstream file(filename);
        if (!file)
        {
            std::cerr << "Erro ao abrir arquivo: " << filename << std::endl;
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
