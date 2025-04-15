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

void parseGroup(XMLElement* groupElement, Group& group){
    XMLElement* transformElement = groupElement->FirstChildElement("transform");
    if (transformElement) {
        parseTransformation(transformElement, group);
    }

    XMLElement* modelsElement = groupElement->FirstChildElement("models");
    if (modelsElement) {
        for (XMLElement* modelElement = modelsElement->FirstChildElement("model"); modelElement; modelElement = modelElement->NextSiblingElement("model")) {
            const char* file = modelElement->Attribute("file");
            if (file) {
                group.models.push_back(file);
            }
        }
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
        cout << indent << "Model: " << model << endl;
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
