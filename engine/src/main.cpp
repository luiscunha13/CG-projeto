#include <iostream>
#include <fstream>
#include <string>
#include <tinyxml2.h>
#include <World.h>

using namespace std;
using namespace tinyxml2;

int a(string filepath) {
    XMLDocument doc;
    XMLError result = doc.LoadFile(filepath.c_str());

    if (result != XML_SUCCESS) {
        cerr << "Failed to load XML file: " << doc.ErrorStr() << endl;
        return -1;
    }

     World world;

    // Get root element
    XMLElement* rootElement = doc.RootElement();
    if (!rootElement) {
        std::cerr << "No root element found" << std::endl;
        return -1;
    }

    // Parse window element
    XMLElement* windowElement = rootElement->FirstChildElement("window");
    if (windowElement) {
        windowElement->QueryIntAttribute("width", &world.window.width);
        windowElement->QueryIntAttribute("height", &world.window.height);
    }

    // Parse camera element
    XMLElement* cameraElement = rootElement->FirstChildElement("camera");
    if (cameraElement) {
        // Parse position
        XMLElement* positionElement = cameraElement->FirstChildElement("position");
        if (positionElement) {
            positionElement->QueryFloatAttribute("x", &world.camera.position.x);
            positionElement->QueryFloatAttribute("y", &world.camera.position.y);
            positionElement->QueryFloatAttribute("z", &world.camera.position.z);
        }

        // Parse lookAt
        XMLElement* lookAtElement = cameraElement->FirstChildElement("lookAt");
        if (lookAtElement) {
            lookAtElement->QueryFloatAttribute("x", &world.camera.lookAt.x);
            lookAtElement->QueryFloatAttribute("y", &world.camera.lookAt.y);
            lookAtElement->QueryFloatAttribute("z", &world.camera.lookAt.z);
        }

        // Parse up vector
        XMLElement* upElement = cameraElement->FirstChildElement("up");
        if (upElement) {
            upElement->QueryFloatAttribute("x", &world.camera.up.x);
            upElement->QueryFloatAttribute("y", &world.camera.up.y);
            upElement->QueryFloatAttribute("z", &world.camera.up.z);
        }

        // Parse projection
        XMLElement* projectionElement = cameraElement->FirstChildElement("projection");
        if (projectionElement) {
            projectionElement->QueryFloatAttribute("fov", &world.camera.projection.fov);
            projectionElement->QueryFloatAttribute("near", &world.camera.projection.near);
            projectionElement->QueryFloatAttribute("far", &world.camera.projection.far);
        }
    }

    // Parse models
    XMLElement* groupElement = rootElement->FirstChildElement("group");
    if (groupElement) {
        XMLElement* modelsElement = groupElement->FirstChildElement("models");
        if (modelsElement) {
            for (XMLElement* modelElement = modelsElement->FirstChildElement("model");
                 modelElement != nullptr;
                 modelElement = modelElement->NextSiblingElement("model")) {
                Model model;
                const char* file = modelElement->Attribute("file");
                if (file) {
                    model.file = file;
                    world.models.push_back(model);
                }
            }
        }
    }

    // Print out the parsed data to verify
    std::cout << "Window: " << world.window.width << "x" << world.window.height << std::endl;

    std::cout << "Camera:" << std::endl;
    std::cout << "  Position: (" << world.camera.position.x << ", "
              << world.camera.position.y << ", " << world.camera.position.z << ")" << std::endl;
    std::cout << "  LookAt: (" << world.camera.lookAt.x << ", "
              << world.camera.lookAt.y << ", " << world.camera.lookAt.z << ")" << std::endl;
    std::cout << "  Up: (" << world.camera.up.x << ", "
              << world.camera.up.y << ", " << world.camera.up.z << ")" << std::endl;
    std::cout << "  Projection: FOV=" << world.camera.projection.fov
              << ", Near=" << world.camera.projection.near
              << ", Far=" << world.camera.projection.far << std::endl;

    std::cout << "Models:" << std::endl;
    for (const Model& model : world.models) {
        std::cout << "  " << model.file << std::endl;
    }

    return 0;
}

int main(const int argc, char *argv[]){
    if (argc < 2)
    {
        cerr << "Usage: engine <scene.xml>" << std::endl;
        return 1;
    }

    const string filepath = argv[1];
    a(filepath);
    return 0;

}
