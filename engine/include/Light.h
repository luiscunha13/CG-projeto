#ifndef LIGHT_H
#define LIGHT_H

#include <vertex.h>
#include <string>

//o Light.h foi adicionado para esta fase
struct Color {
    float r, g, b;

    Color() : r(0.0f), g(0.0f), b(0.0f) {}
    Color(float r, float g, float b) : r(r), g(g), b(b) {}

    // Convert from 0-255 range to 0.0-1.0 range
    static Color fromRGB(int R, int G, int B) {
        return Color(R / 255.0f, G / 255.0f, B / 255.0f);
    }
};

struct Material {
    Color diffuse;  // Default: 200, 200, 200
    Color ambient;  // Default: 50, 50, 50
    Color specular; // Default: 0, 0, 0
    Color emissive; // Default: 0, 0, 0
    float shininess; // Default: 0

    Material() :
        diffuse(Color::fromRGB(200, 200, 200)),
        ambient(Color::fromRGB(50, 50, 50)),
        specular(0.0f, 0.0f, 0.0f),
        emissive(0.0f, 0.0f, 0.0f),
        shininess(0.0f) {}
};

struct Texture {
    std::string filename;
    GLuint textureID;
    bool loaded;

    Texture() : filename(""), textureID(0), loaded(false) {}
    explicit Texture(const std::string& file) : filename(file), textureID(0), loaded(false) {}
};

struct Light {
    enum class Type {
        Point,
        Directional,
        Spotlight
    };

    Type type;
    Vertex3f position;  // For point and spotlight
    Vertex3f direction; // For directional and spotlight
    float cutoff;       // For spotlight (in degrees)

    // Light properties (could be expanded)
    Color ambient;
    Color diffuse;
    Color specular;

    Light() :
        type(Type::Point),
        position(0.0f, 0.0f, 0.0f),
        direction(0.0f, 0.0f, 1.0f),
        cutoff(45.0f),
        ambient(0.2f, 0.2f, 0.2f),
        diffuse(1.0f, 1.0f, 1.0f),
        specular(1.0f, 1.0f, 1.0f) {}
};

#endif // LIGHT_H
