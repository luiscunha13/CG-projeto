#include <generator.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const string commands[] = {
    "generator plane <size> <divisions> <filename>",
    "generator box <size> <divisions> <filename>",
    "generator sphere <radius> <slices> <stacks> <filename>",
    "generator cone <radius> <height> <slices> <stacks> <filename>",
    "generator cylinder <radius> <height> <slices> <filename>",
    "generator torus <inner_radius> <outer_radius> <slices> <stacks> <filename>",
    "generator bezier <patch_file> <tessellation> <filename>",
};

void print_usage() {
    cout << "Usage:" << endl;
    for (const string &command : commands) {
        cout << "  " << command << endl;
    }
}

void print_command_usage(const string &figure) {
    if (figure == "plane") {
        cout << "Invalid usage." << endl;
        cout << commands[0] << endl;
    } else if (figure == "box") {
        cout << commands[1] << endl;
    } else if (figure == "sphere") {
        cout << commands[2] << endl;
    } else if (figure == "cone") {
        cout << commands[3] << endl;
    } else if (figure == "cylinder") {
        cout << commands[4] << endl;
    } else if (figure == "torus") {
        cout << commands[5] << endl;
    }
    else if(figure == "bezier") {
        cout << commands[6] << endl;
    }
    else {
        cout << "Invalid figure. ";
        print_usage();
    }
}


int main(const int argc, char *argv[]){
    if (argc < 2) {
        print_usage();
        return 1;
    }

    string figure = argv[1];

    if (figure == "plane") {
        if (argc != 5) {
            print_command_usage(figure);
            return 1;
        }

        try {
            float size = stof(argv[2]);
            int divisions = stof(argv[3]);
            SaveModel(generator::generatePlane(size, divisions), argv[4]);
        }
        catch (const std::logic_error &) {
            cerr << "Error parsing the arguments of " << figure << endl;
            return 1;
        }
    } else if (figure == "box") {
        if (argc != 5) {
            print_command_usage(figure);
            return 1;
        }

        try {
            float lenght = stof(argv[2]);
            int divisions = stoi(argv[3]);
            SaveModel(generator::generateBox(lenght, divisions), argv[4]);
            cout << "Successfully saved the Box model." << endl;
        }
        catch (const std::logic_error &) {
            cerr << "Error parsing the arguments of " << figure << endl;
            return 1;
        }
    } else if (figure == "sphere") {
        if (argc != 6) {
            print_command_usage(figure);
            return 1;
        }

        try {
            float radius = stof(argv[2]);
            int slices = stoi(argv[3]);
            int stacks = stoi(argv[4]);
            SaveModel(generator::generateSphere(radius, slices, stacks), argv[5]);
            cout << "Successfully saved the Sphere model." << endl;
        }
        catch (const std::logic_error &) {
            cerr << "Error parsing the arguments of " << figure << endl;
            return 1;
        }
    } else if (figure == "cone") {
        if (argc != 7) {
            print_command_usage(figure);
            return 1;
        }

        try {
            float radius = stof(argv[2]);
            float height = stof(argv[3]);
            unsigned int slices = stoi(argv[4]);
            unsigned int stacks = stoi(argv[5]);
            SaveModel(generator::generateCone(radius, height, slices, stacks), argv[6]);
            cout << "Successfully saved the Cone model." << endl;
        }
        catch (const std::logic_error &) {
            cerr << "Error parsing the arguments of " << figure << endl;
            return 1;
        }
    } else if (figure == "cylinder") {
        if (argc != 6) {
            print_command_usage(figure);
            return 1;
        }

        try {
            float radius = stof(argv[2]);
            float height = stof(argv[3]);
            unsigned int slices = stoi(argv[4]);
            SaveModel(generator::generateCylinder(radius, height, slices), argv[5]);
            cout << "Successfully saved the Cylinder model." << endl;
        }
        catch (const std::logic_error &) {
            cerr << "Error parsing the arguments of " << figure << endl;
            return 1;
        }
    } else if (figure == "torus") {
        if (argc != 7) {
            print_command_usage(figure);
            return 1;
        }

        try {
            float inner_radius = stof(argv[2]);
            float outer_radius = stof(argv[3]);
            unsigned int slices = stoi(argv[4]);
            unsigned int stacks = stoi(argv[5]);
            SaveModel(generator::generateTorus(outer_radius, inner_radius, slices, stacks), argv[6]);
            cout << "Successfully saved the Torus model." << endl;
        }
        catch (const std::logic_error &) {
            cerr << "Error parsing the arguments of " << figure << endl;
            return 1;
        }
    } else if (figure == "bezier") {
        if (argc != 5) {
            print_command_usage(figure);
            return 1;
        }

        try {
            string patchFile = argv[2];
            int tessellation = stoi(argv[3]);
            SaveModel(generator::generateBezier(patchFile, tessellation), argv[4]);
            cout << "Successfully saved the Bezier patch model." << endl;
        }
        catch (const std::logic_error &) {
            cerr << "Error parsing the arguments of " << figure << endl;
            return 1;
        }
    } else {
        print_command_usage("");
    }

    return 0;
}