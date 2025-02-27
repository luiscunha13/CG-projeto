#ifndef VEC_H
#define VEC_H

#include <cmath>

// Classe para vetores 2D
class Vec2f {
  public:
    float x,y;

    Vec2f() : x(0.0f), y(0.0f) {}
    Vec2f(float x, float y) : x(x), y(y) {}

    Vec2f operator+(const Vec2f& v) const {
      return Vec2f(x + v.x, y + v.y);
    }

    Vec2f operator-(const Vec2f& v) const {
      return Vec2f(x - v.x, y - v.y);
    }

    Vec2f operator*(float f) const {
      return Vec2f(x * f, y * f);
    }

    Vec2f operator/(float f) const {
      float invF = 1.0f / f;
      return Vec2f(x * invF, y * invF);
    }

    // produto escalar
    float dot(const Vec2f& v) const {
      return x * v.x + y * v.y;
    }

    // comprimento ao quadrado
    float lengthSquared() const {
      return x * x + y * y;
    }

    //comprimento
    float length() const {
      return sqrtf(lengthSquared());
    }

    Vec2f normalize() const {
      float len = length();
      if(len > 0) {
        return *this / len;
        }
        return *this;
    }
};

// classe para vetores 3D
class Vec3f {
  public:
    float x,y,z;

    Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3f operator+(const Vec3f& v) const {
      return Vec3f(x + v.x, y + v.y, z + v.z);
    }

    Vec3f operator-(const Vec3f& v) const {
      return Vec3f(x - v.x, y - v.y, z - v.z);
    }

    Vec3f operator*(float f) const {
      return Vec3f(x * f, y * f, z * f);
    }

    Vec3f operator/(float f) const {
      float invF = 1.0f / f;
      return Vec3f(x * invF, y * invF, z * invF);
    }

    bool operator==(const Vec3f& v) const {
      return x == v.x && y == v.y && z == v.z;
    }

    // Produto escalar (dot product)
    float dot(const Vec3f& v) const {
      return x * v.x + y * v.y + z * v.z;
    }

    // Produto vetorial (cross product)
    Vec3f cross(const Vec3f& v) const {
      return Vec3f(
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
    Vec3f normalize() const {
        float len = length();
        if (len > 0) {
          return *this / len;
        }
        return *this;
      }
};

// Operadores para escalar * vetor
inline Vec2f operator*(float scalar, const Vec2f& v) {
  return v * scalar;
}

inline Vec3f operator*(float scalar, const Vec3f& v) {
  return v * scalar;
}

#endif // VEC_H