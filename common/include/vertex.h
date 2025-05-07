#ifndef VERTEX_H
#define VERTEX_H

#include <cmath>

struct Vertex3f {
    float x,y,z;
    float nx, ny, nz; // Normal
    float s, t;       // Textura

    Vertex3f() : x(0.0f), y(0.0f), z(0.0f) {}
    Vertex3f(float x, float y, float z, float nx = 0, float ny = 0, float nz = 0, float s = 0, float t = 0)
            : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz), s(s), t(t) {}

    Vertex3f operator+(const Vertex3f& v) const {
       return Vertex3f(x + v.x, y + v.y, z + v.z);
    }

    Vertex3f operator-(const Vertex3f& v) const {
       return Vertex3f(x - v.x, y - v.y, z - v.z);
    }

    Vertex3f operator*(float f) const {
       return Vertex3f(x * f, y * f, z * f);
    }

    Vertex3f operator/(float f) const {
       float invF = 1.0f / f;
       return Vertex3f(x * invF, y * invF, z * invF);
    }

    bool operator==(const Vertex3f& v) const {
         return x == v.x && y == v.y && z == v.z;
    }

    // Produto escalar (dot product)
    float dot(const Vertex3f& v) const {
         return x * v.x + y * v.y + z * v.z;
    }

    // Produto vetorial (cross product)
    Vertex3f cross(const Vertex3f& v) const {
        return Vertex3f(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    // Comprimento ao quadrado
    float lengthSquared() const {
        return x * x + y * y + z * z;
    }

    // Comprimento
    float length() const {
        return std::sqrt(lengthSquared());
    }

    // Normalizar
    Vertex3f normalize() const {
        float len = length();
        if (len > 0) {
            return *this / len;
        }
        return *this;
    }
};

inline Vertex3f operator*(float scalar, const Vertex3f& v) {
  return v * scalar;
}
//adicionado para o texCoord
struct Vertex2f {
    float x,y;
    Vertex2f() : x(0.0f), y(0.0f) {}
    Vertex2f(float x, float y) : x(x), y(y) {}
};

#endif // VERTEX_H