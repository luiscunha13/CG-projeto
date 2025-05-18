#include <iostream>
#include <fstream>
#include <string>
#include <tinyxml2.h>
#include <World.h>
#include "engine.h"

using namespace std;
using namespace tinyxml2;

void parseTransformation(XMLElement* transformElement, Group& group) {
    for (XMLElement* element = transformElement->FirstChildElement(); element; element = element->NextSiblingElement()) {
        Transformation transformation;
        if (strcmp(element->Name(), "translate") == 0) {
            transformation.type = Transformation::Type::Translate;
            if (element->QueryFloatAttribute("time", &transformation.animation.time) == XML_SUCCESS) {
                // Time-based
                transformation.animated = true;
                transformation.animation.align = element->BoolAttribute("align", false);
                const char* algorithmAttr = element->Attribute("algorithm");
                transformation.animation.algorithm = algorithmAttr ? std::string(algorithmAttr) : "catmull-rom";

                for (XMLElement* pointElement = element->FirstChildElement("point");
                     pointElement;
                     pointElement = pointElement->NextSiblingElement("point")) {

                    Vertex3f point;
                    pointElement->QueryFloatAttribute("x", &point.x);
                    pointElement->QueryFloatAttribute("y", &point.y);
                    pointElement->QueryFloatAttribute("z", &point.z);
                    transformation.animation.points.push_back(point);
                }
            } else {
                // Static
                transformation.animated = false;
                element->QueryFloatAttribute("x", &transformation.coords.x);
                element->QueryFloatAttribute("y", &transformation.coords.y);
                element->QueryFloatAttribute("z", &transformation.coords.z);
            }
        } else if (strcmp(element->Name(), "rotate") == 0) {
            transformation.type = Transformation::Type::Rotate;
            if (element->QueryFloatAttribute("time", &transformation.animation.time) == XML_SUCCESS) {
                // Time-based
                transformation.animated = true;
                element->QueryFloatAttribute("x", &transformation.coords.x);
                element->QueryFloatAttribute("y", &transformation.coords.y);
                element->QueryFloatAttribute("z", &transformation.coords.z);
            } else {
                // Static
                transformation.animated = false;
                element->QueryFloatAttribute("angle", &transformation.angle);
                element->QueryFloatAttribute("x", &transformation.coords.x);
                element->QueryFloatAttribute("y", &transformation.coords.y);
                element->QueryFloatAttribute("z", &transformation.coords.z);
            }
        } else if (strcmp(element->Name(), "scale") == 0) {
            transformation.type = Transformation::Type::Scale;
            element->QueryFloatAttribute("x", &transformation.coords.x);
            element->QueryFloatAttribute("y", &transformation.coords.y);
            element->QueryFloatAttribute("z", &transformation.coords.z);
        }
        group.transformations.push_back(transformation);
    }
}

void parseModels(XMLElement* modelsElement, Group& group) {



    Material defaultMaterial;
    defaultMaterial.diffuse = Vertex3f(200/255.0f, 200/255.0f, 200/255.0f);
    defaultMaterial.ambient = Vertex3f(50/255.0f, 50/255.0f, 50/255.0f);
    defaultMaterial.specular = Vertex3f(0, 0, 0);
    defaultMaterial.emissive = Vertex3f(0, 0, 0);
    defaultMaterial.shininess = 0;

    for (XMLElement* modelElement = modelsElement->FirstChildElement("model");
         modelElement;
         modelElement = modelElement->NextSiblingElement("model")) {

        ModelInfo modelInfo;
        const char* file = modelElement->Attribute("file");
        if (file) modelInfo.file = file;

        modelInfo.isSkybox = modelElement->BoolAttribute("skybox", false);

        modelInfo.material = defaultMaterial;

        XMLElement* textureElement = modelElement->FirstChildElement("texture");
        if (textureElement) {
            const char* textureFile = textureElement->Attribute("file");
            if (textureFile) modelInfo.texture = textureFile;
        }

        XMLElement* colorElement = modelElement->FirstChildElement("color");
        if (colorElement) {
            XMLElement* diffuseElement = colorElement->FirstChildElement("diffuse");
            if (diffuseElement) {
                int r = 200, g = 200, b = 200;
                diffuseElement->QueryIntAttribute("R", &r);
                diffuseElement->QueryIntAttribute("G", &g);
                diffuseElement->QueryIntAttribute("B", &b);
                // Convert from 0-255 to 0.0-1.0
                modelInfo.material.diffuse.x = r / 255.0f;
                modelInfo.material.diffuse.y = g / 255.0f;
                modelInfo.material.diffuse.z = b / 255.0f;
            }

            XMLElement* ambientElement = colorElement->FirstChildElement("ambient");
            if (ambientElement) {
                int r = 50, g = 50, b = 50;
                ambientElement->QueryIntAttribute("R", &r);
                ambientElement->QueryIntAttribute("G", &g);
                ambientElement->QueryIntAttribute("B", &b);
                // Convert from 0-255 to 0.0-1.0
                modelInfo.material.ambient.x = r / 255.0f;
                modelInfo.material.ambient.y = g / 255.0f;
                modelInfo.material.ambient.z = b / 255.0f;
            }

            XMLElement* specularElement = colorElement->FirstChildElement("specular");
            if (specularElement) {
                int r = 0, g = 0, b = 0;
                specularElement->QueryIntAttribute("R", &r);
                specularElement->QueryIntAttribute("G", &g);
                specularElement->QueryIntAttribute("B", &b);
                // Convert from 0-255 to 0.0-1.0
                modelInfo.material.specular.x = r / 255.0f;
                modelInfo.material.specular.y = g / 255.0f;
                modelInfo.material.specular.z = b / 255.0f;
            }

            XMLElement* emissiveElement = colorElement->FirstChildElement("emissive");
            if (emissiveElement){
                int r = 0, g = 0, b = 0;
                emissiveElement->QueryIntAttribute("R", &r);
                emissiveElement->QueryIntAttribute("G", &g);
                emissiveElement->QueryIntAttribute("B", &b);
                // Convert from 0-255 to 0.0-1.0
                modelInfo.material.emissive.x = r / 255.0f;
                modelInfo.material.emissive.y = g / 255.0f;
                modelInfo.material.emissive.z = b / 255.0f;
            }

            XMLElement* shininessElement = colorElement->FirstChildElement("shininess");
            if (shininessElement) {
                float value = 0;
                shininessElement->QueryFloatAttribute("value", &value);
                modelInfo.material.shininess = value;
            }
        }

        group.models.push_back(modelInfo);
    }
}

void parseGroup(XMLElement* groupElement, Group& group){
    XMLElement* transformElement = groupElement->FirstChildElement("transform");
    if (transformElement) {
        parseTransformation(transformElement, group);
    }

    XMLElement* modelsElement = groupElement->FirstChildElement("models");
    if (modelsElement) {
        parseModels(modelsElement, group);
    }

    for (XMLElement* childGroupElement = groupElement->FirstChildElement("group"); childGroupElement; childGroupElement = childGroupElement->NextSiblingElement("group")) {
        Group childGroup;
        parseGroup(childGroupElement, childGroup);
        group.childGroups.push_back(childGroup);
    }
}

void printGroup(const Group& group, int depth = 0) {
    string indent(depth * 2, ' ');

    for (const auto& model : group.models) {
        cout << indent << "Model: " << model.file << endl;
        cout << indent << "Texture: " << model.texture << endl;
        cout << indent << "Color: " << endl;
        cout << indent << "Diffuse: R:" << model.material.diffuse.x << ", G: " << model.material.diffuse.y << ", B: " << model.material.diffuse.z << endl;
        cout << indent << "Ambient: R:" << model.material.ambient.x << ", G: " << model.material.ambient.y << ", B: " << model.material.ambient.z << endl;
        cout << indent << "Specular: R:" << model.material.specular.x << ", G: " << model.material.specular.y << ", B: " << model.material.specular.z << endl;
        cout << indent << "Emissive: R:" << model.material.emissive.x << ", G: " << model.material.emissive.y << ", B: " << model.material.emissive.z << endl;
        cout << indent << "Shininess: " << model.material.shininess << endl;
    }

    for (const auto& transformation : group.transformations) {
        cout << indent << "Transformation: ";
        switch (transformation.type) {
            case Transformation::Type::Translate:
                if (transformation.animated) {
                    cout << "Animated Translate (time=" << transformation.animation.time
                         << "s, align=" << (transformation.animation.align ? "true" : "false")
                         << ", points=[";
                    for (const auto& point : transformation.animation.points) {
                        cout << "(" << point.x << "," << point.y << "," << point.z << ") ";
                    }
                    cout << "])";
                } else {
                    cout << "Translate (" << transformation.coords.x << ", "
                         << transformation.coords.y << ", " << transformation.coords.z << ")";
                }
                break;
            case Transformation::Type::Rotate:
                if (transformation.animated) {
                    cout << "Animated Rotate (time=" << transformation.animation.time
                         << "s, axis=(" << transformation.coords.x << ", "
                         << transformation.coords.y << ", " << transformation.coords.z << "))";
                } else {
                    cout << "Rotate (angle=" << transformation.angle
                         << ", axis=(" << transformation.coords.x << ", "
                         << transformation.coords.y << ", " << transformation.coords.z << "))";
                }
                break;
            case Transformation::Type::Scale:
                cout << "Scale (" << transformation.coords.x << ", "
                     << transformation.coords.y << ", " << transformation.coords.z << ")";
                break;
        }
        cout << endl;
    }

    for (const auto& childGroup : group.childGroups) {
        cout << indent << "Child Group:" << endl;
        printGroup(childGroup, depth + 1);
    }
}

World *parse_scene(string filepath) {
    XMLDocument doc;
    XMLError result = doc.LoadFile(filepath.c_str());

    if (result != XML_SUCCESS) {
        cerr << "Failed to load XML file: " << doc.ErrorStr() << endl;
        return nullptr;
    }

    World *world = new World();

    XMLElement* rootElement = doc.RootElement();
    if (!rootElement) {
        std::cerr << "No root element found" << std::endl;
        return nullptr;
    }

    XMLElement* windowElement = rootElement->FirstChildElement("window");
    if (windowElement) {
        windowElement->QueryIntAttribute("width", &world->window.width);
        windowElement->QueryIntAttribute("height", &world->window.height);
    }

    XMLElement* cameraElement = rootElement->FirstChildElement("camera");
    if (cameraElement) {
        XMLElement* positionElement = cameraElement->FirstChildElement("position");
        if (positionElement) {
            positionElement->QueryFloatAttribute("x", &world->camera.position.x);
            positionElement->QueryFloatAttribute("y", &world->camera.position.y);
            positionElement->QueryFloatAttribute("z", &world->camera.position.z);
        }

        XMLElement* lookAtElement = cameraElement->FirstChildElement("lookAt");
        if (lookAtElement) {
            lookAtElement->QueryFloatAttribute("x", &world->camera.lookAt.x);
            lookAtElement->QueryFloatAttribute("y", &world->camera.lookAt.y);
            lookAtElement->QueryFloatAttribute("z", &world->camera.lookAt.z);
        }

        XMLElement* upElement = cameraElement->FirstChildElement("up");
        if (upElement) {
            upElement->QueryFloatAttribute("x", &world->camera.up.x);
            upElement->QueryFloatAttribute("y", &world->camera.up.y);
            upElement->QueryFloatAttribute("z", &world->camera.up.z);
        }

        XMLElement* projectionElement = cameraElement->FirstChildElement("projection");
        if (projectionElement) {
            projectionElement->QueryFloatAttribute("fov", &world->camera.projection.fov);
            projectionElement->QueryFloatAttribute("near", &world->camera.projection.near);
            projectionElement->QueryFloatAttribute("far", &world->camera.projection.far);
        }
    }

    XMLElement* lightsElement = rootElement->FirstChildElement("lights");
    if (lightsElement) {
        for (XMLElement* lightElement = lightsElement->FirstChildElement("light");
             lightElement;
             lightElement = lightElement->NextSiblingElement("light")) {

            Light light;
            const char* type = lightElement->Attribute("type");
            if (strcmp(type, "point") == 0) light.type = Light::POINT;
            else if (strcmp(type, "directional") == 0) light.type = Light::DIRECTIONAL;
            else if (strcmp(type, "spotlight") == 0) light.type = Light::SPOT;

            lightElement->QueryFloatAttribute("posX", &light.position.x);
            lightElement->QueryFloatAttribute("posY", &light.position.y);
            lightElement->QueryFloatAttribute("posZ", &light.position.z);

            lightElement->QueryFloatAttribute("dirX", &light.direction.x);
            lightElement->QueryFloatAttribute("dirY", &light.direction.y);
            lightElement->QueryFloatAttribute("dirZ", &light.direction.z);

            lightElement->QueryFloatAttribute("cutoff", &light.cutoff);

            world->lights.push_back(light);
        }
    }


    for(XMLElement* groupElement = rootElement->FirstChildElement("group"); groupElement; groupElement = groupElement->NextSiblingElement("group")){
        Group group;
        parseGroup(groupElement, group);
        world->groups.push_back(group);
    }


    std::cout << "Window: " << world->window.width << "x" << world->window.height << std::endl;

    std::cout << "Camera:" << std::endl;
    std::cout << "  Position: (" << world->camera.position.x << ", "
              << world->camera.position.y << ", " << world->camera.position.z << ")" << std::endl;
    std::cout << "  LookAt: (" << world->camera.lookAt.x << ", "
              << world->camera.lookAt.y << ", " << world->camera.lookAt.z << ")" << std::endl;
    std::cout << "  Up: (" << world->camera.up.x << ", "
              << world->camera.up.y << ", " << world->camera.up.z << ")" << std::endl;
    std::cout << "  Projection: FOV=" << world->camera.projection.fov
              << ", Near=" << world->camera.projection.near
              << ", Far=" << world->camera.projection.far << std::endl;
    std::cout << "Lights:" << std::endl;

    for(Light l : world->lights)
        if(l.type == Light :: POINT)
            std::cout << "type: Point" << ", posX:" << l.position.x
                      << ", posY:" << l.position.y << ", posZ:" << l.position.z << std::endl;
        else if(l.type == Light :: DIRECTIONAL)
            std::cout << "type: Directional" << ", dirX:" << l.direction.x
                      << ", dirY:" << l.direction.y << ", dirZ:" << l.direction.z << std::endl;
        else
            std::cout << "type: Spotlight" << ", posX:" << l.position.x
                      << ", posY:" << l.position.y << ", posZ:" << l.position.z
                      << ", dirX:" << l.direction.x << ", dirY:" << l.direction.y
                      << ", dirZ:" << l.direction.z << ", cutoff:" << l.cutoff << std::endl;



    std::cout << "Groups:" << std::endl;
    for (const auto& group : world->groups) {
        cout << "Group:" << endl;
        printGroup(group, 1);
    }

    cout << endl;

    return world;
}

int main(const int argc, char *argv[]){
    if (argc < 2)
    {
        cerr << "Usage: engine <scene.xml>" << std::endl;
        return 1;
    }

    const string filepath = argv[1];
    World *world = parse_scene(filepath);
    if (world == nullptr) return 1;
    run_engine(*world, argc, argv);
    return 0;

}
