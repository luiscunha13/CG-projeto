#include <generator.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const string commands[] = {
    "generator plane <size> <divisions> <filename>",
    "generator box <size> <divisions> <filename>",
    "generator sphere <radius> <slices> <stacks> <filename>",
    "generator cone <radius> <height> <slices> <stacks> <filename>"
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
    } else {
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
    } else if (figure == "box") {
        if (argc != 5) {
            print_command_usage(figure);
            return 1;
        }
    } else if (figure == "sphere") {
        if (argc != 6) {
            print_command_usage(figure);
            return 1;
        }
    } else if (figure == "cone") {
        if (argc != 7) {
            print_command_usage(figure);
            return 1;
        }
    } else {
        print_command_usage("");
    }
}