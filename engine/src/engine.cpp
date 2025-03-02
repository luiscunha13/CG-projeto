#include <engine.h>
#include <fstream>
#include <iostream>
#include <Model.h>
#include <vector>
#include <string>
#include <GL/glut.h>

World world = {};
std::vector<Model> models;

float translateX = 0.0f;
float translateZ = 0.0f;
float rotationAngle = 0.0f;
float height = 2.0f;

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if(h == 0)
		h = 1;

	// compute window's aspect ratio
	float ratio = w * 1.0 / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void renderScene() {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(world.camera.position.x,world.camera.position.y,world.camera.position.z,
              world.camera.lookAt.x, world.camera.lookAt.y, world.camera.lookAt.z,
              world.camera.up.x,world.camera.up.y,world.camera.up.z);

    //glTranslatef(translateX, 0.0f, translateZ);
    //glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    //desenhar eixos
    /*glBegin(GL_LINES);
    // X Red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f( 100.0f, 0.0f, 0.0f);
    // Y Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);
    // Z Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();*/

	for (const auto& model : models) {
		glBegin(GL_TRIANGLES);

		// Process each triangle
		for (int i = 0; i < model.n_indices; i += 3) {
			// Get indices for the three vertices of this triangle
			unsigned int idx1 = model.indices[i];
			unsigned int idx2 = model.indices[i + 1];
			unsigned int idx3 = model.indices[i + 2];

			// Get vertices
			const Vertex3f& v1 = model.vertices[idx1];
			const Vertex3f& v2 = model.vertices[idx2];
			const Vertex3f& v3 = model.vertices[idx3];

			// Draw the triangle
			glVertex3f(v1.x, v1.y, v1.z);
			glVertex3f(v2.x, v2.y, v2.z);
			glVertex3f(v3.x, v3.y, v3.z);
		}

		// End triangle drawing
		glEnd();
	}



    // put pyramid drawing instructions here


    // End of frame
    glutSwapBuffers();
}

void processKeys(unsigned char key, int x, int y){
	switch (key) {
		case 'd': // when 'd' is pressed
			translateX += 0.1f; // increment X position
		glutPostRedisplay(); // request a redraw
		break;
		case 'a':
			translateX -= 0.1f; // increment X position
		glutPostRedisplay(); // request a redraw
		break;
		case 'w': // when 'd' is pressed
			translateZ -= 0.1f; // increment X position
		glutPostRedisplay(); // request a redraw
		break;
		case 's':
			translateZ += 0.1f; // increment X position
		glutPostRedisplay(); // request a redraw
		break;
	}
}

void processKeysSpecial(int key, int x, int y){
	switch (key) {
		case GLUT_KEY_UP: // when 'd' is pressed
			height += 0.1f; // increment X position
		glutPostRedisplay(); // request a redraw
		break;
		case GLUT_KEY_DOWN:
			height -= 0.1f; // increment X position
		glutPostRedisplay(); // request a redraw
		break;
		case GLUT_KEY_RIGHT: // when 'd' is pressed
			rotationAngle += 5.0f; // increment X position
		glutPostRedisplay(); // request a redraw
		break;
		case GLUT_KEY_LEFT:
			rotationAngle -= 5.0f; // increment X position
		glutPostRedisplay(); // request a redraw
		break;
	}
}


bool readModelFromFile(const std::string& filename, Model& model) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    // Read number of vertices and triangles
    int n_triangles;
    if (!(file >> model.n_vertices >> n_triangles)) {
        std::cerr << "Error reading vertex and triangle counts from " << filename << std::endl;
        file.close();
        return false;
    }

    model.n_indices = n_triangles * 3; // Convert triangles to indices

    // Resize the vertices vector
    model.vertices.resize(model.n_vertices);

    // Read vertices
    for (int i = 0; i < model.n_vertices; i++) {
        if (!(file >> model.vertices[i].x >> model.vertices[i].y >> model.vertices[i].z)) {
            std::cerr << "Error reading vertex " << i << " from " << filename << std::endl;
            file.close();
            return false;
        }
    }

    // Resize the indices vector
    model.indices.resize(model.n_indices);

    // Read indices
    for (int i = 0; i < model.n_indices; i++) {
        if (!(file >> model.indices[i])) {
            std::cerr << "Error reading index " << i << " from " << filename << std::endl;
            file.close();
            return false;
        }
    }

    file.close();
    return true;
}

void loadModels() {
	for (const auto& filename : world.models) {
		Model model;
		if (readModelFromFile(filename, model)) {
			models.push_back(model);
			std::cout << "Successfully loaded model from " << filename << std::endl;
		} else {
			std::cerr << "Failed to load model from " << filename << std::endl;
		}
	}
}

void run_engine(World new_world, int argc, char **argv) {
	world = new_world;
	loadModels();
    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800,800);
    glutCreateWindow("CG@DI-UM");

    // Required callback registry
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

    // put here the registration of the keyboard callbacks
	glutKeyboardFunc(processKeys);
	glutSpecialFunc(processKeysSpecial);

    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_LINE);

    // enter GLUT's main cycle
    glutMainLoop();
}