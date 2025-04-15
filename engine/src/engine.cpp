#include <engine.h>
#include <fstream>
#include <iostream>
#include <Model.h>
#include <vector>
#include <string>
#include <GL/glut.h>
#include <unordered_set>

World world = {};
std::vector<Model> models;

float camX, camY, camZ;
float lookAtX, lookAtY, lookAtZ;
float upX, upY, upZ;

float hAngle = 0.0f;   // Horizontal angle (left/right)
float vAngle = 0.0f; // Vertical angle (up/down)
float speed = 1.0f; // Movement speed

float fov = 45.0f;

bool mouseLeftDown = false;
int lastMouseX = -1, lastMouseY = -1;

void changeSize(int w, int h) {

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
	gluPerspective(fov ,ratio, 1.0f ,1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void renderScene() {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(camX,camY,camZ,
              lookAtX, lookAtY, lookAtZ,
              upX, upY, upZ);

    //desenhar eixos
    glBegin(GL_LINES);
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
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();

	for (const auto& model : models) {
        glPushMatrix();

        for (const auto& transformation : model.transformations) {
            switch (transformation.type) {
                case Transformation::Type::Translate:
                    glTranslatef(transformation.coords.x, transformation.coords.y, transformation.coords.z);
                    break;
                case Transformation::Type::Rotate:
                    glRotatef(transformation.angle, transformation.coords.x, transformation.coords.y, transformation.coords.z);
                    break;
                case Transformation::Type::Scale:
                    glScalef(transformation.coords.x, transformation.coords.y, transformation.coords.z);
                    break;
            }
        }

		glBegin(GL_TRIANGLES);

		for (int i = 0; i < model.n_indices; i += 3) {
			unsigned int idx1 = model.indices[i];
			unsigned int idx2 = model.indices[i + 1];
			unsigned int idx3 = model.indices[i + 2];

			const Vertex3f& v1 = model.vertices[idx1];
			const Vertex3f& v2 = model.vertices[idx2];
			const Vertex3f& v3 = model.vertices[idx3];

			glVertex3f(v1.x, v1.y, v1.z);
			glVertex3f(v2.x, v2.y, v2.z);
			glVertex3f(v3.x, v3.y, v3.z);
		}

		glEnd();

        glPopMatrix();
	}

    glutSwapBuffers();
}

void initializeCamera() {
    camX = world.camera.position.x;
    camY = world.camera.position.y;
    camZ = world.camera.position.z;

    lookAtX = world.camera.lookAt.x;
    lookAtY = world.camera.lookAt.y;
    lookAtZ = world.camera.lookAt.z;

    upX = world.camera.up.x;
    upY = world.camera.up.y;
    upZ = world.camera.up.z;

    // initial angles
    float dx = lookAtX - camX;
    float dy = lookAtY - camY;
    float dz = lookAtZ - camZ;

    // horizontal angle
    hAngle = atan2(dz, dx);

    // vertical angle
    float horizontalDistance = sqrt(dx*dx + dz*dz);
    vAngle = atan2(dy, horizontalDistance);

    // Print initial camera state for debugging
    std::cout << "Initial Camera Position: ("
              << camX << ", " << camY << ", " << camZ << ")" << std::endl;
    std::cout << "Initial Look-At: ("
              << lookAtX << ", " << lookAtY << ", " << lookAtZ << ")" << std::endl;
    std::cout << "Initial Angles - Horizontal: " << hAngle
              << ", Vertical: " << vAngle << std::endl;
}

void updateCamera() {
    lookAtX = camX + cos(hAngle) * cos(vAngle);
    lookAtY = camY + sin(vAngle);
    lookAtZ = camZ + sin(hAngle) * cos(vAngle);
}

void processKeys(unsigned char key, int x, int y){
    switch (key) {
        case 'w': // Move forward
            camX += speed * cos(hAngle);
            camZ += speed * sin(hAngle);
            break;
        case 's': // Move backward
            camX -= speed * cos(hAngle);
            camZ -= speed * sin(hAngle);
            break;
        case 'a': // Move left
            camX += speed * sin(hAngle);
            camZ -= speed * cos(hAngle);
            break;
        case 'd': // Move right
            camX -= speed * sin(hAngle);
            camZ += speed * cos(hAngle);
            break;
        case 'q': // Move up
            camY += speed;
            break;
        case 'e': // Move down
            camY -= speed;
            break;
        case '+': // Increase movement speed
            speed *= 1.1f;
            std::cout << "Speed: " << speed << std::endl;
            break;
        case '-': // Decrease movement speed
            speed *= 0.9f;
            std::cout << "Speed: " << speed << std::endl;
            break;
    }
    updateCamera();
    glutPostRedisplay();
}

void processMouseMotion(int x, int y) {
    // Only process if the left button is down and we have a valid last position
    if (!mouseLeftDown || lastMouseX == -1 || lastMouseY == -1) {
        return;
    }

    // Calculate movement delta
    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;

    // Very small sensitivity for smoother motion
    const float sensitivity = 0.002f;

    // Apply the changes
    hAngle -= deltaX * sensitivity;
    vAngle += deltaY * sensitivity;

    // Clamp vertical angle
    const float maxVAngle = M_PI / 2.0f - 0.01f;
    if (vAngle > maxVAngle) vAngle = maxVAngle;
    if (vAngle < -maxVAngle) vAngle = -maxVAngle;

    // Save the current position for the next frame
    lastMouseX = x;
    lastMouseY = y;

    // Update the camera view
    updateCamera();
    glutPostRedisplay();
}

void processMouseButtons(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
            // Only initialize lastMouse position when button is first pressed
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mouseLeftDown = false;
            // Important: Reset the last position when the button is released
            lastMouseX = -1;
            lastMouseY = -1;
        }
    }

    // Handle mouse wheel scrolling
    if (state == GLUT_DOWN) {

        if (button == 3) {  // Scroll up - zoom in
            fov -= 2.0f;
            if (fov < 10.0f) fov = 10.0f; // Min FOV
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            int width = glutGet(GLUT_WINDOW_WIDTH);
            int height = glutGet(GLUT_WINDOW_HEIGHT);
            float ratio = width * 1.0f / height;
            gluPerspective(fov, ratio, 0.1f, 1000.0f);
            glMatrixMode(GL_MODELVIEW);

        } else if (button == 4) {  // Scroll down - zoom out
            fov += 2.0f;
            if (fov > 90.0f) fov = 90.0f; // Max FOV
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            int width = glutGet(GLUT_WINDOW_WIDTH);
            int height = glutGet(GLUT_WINDOW_HEIGHT);
            float ratio = width * 1.0f / height;
            gluPerspective(fov, ratio, 0.1f, 1000.0f);
            glMatrixMode(GL_MODELVIEW);
        }
    }
    updateCamera();
    glutPostRedisplay();
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

void loadModelsGroup(const Group& group,  const std::vector<Transformation>& parentTransformations) {

    std::vector<Transformation> transformations = parentTransformations;
    transformations.insert(transformations.end(), group.transformations.begin(), group.transformations.end());

    for (const auto& filename : group.models) {
        Model model;
        if (readModelFromFile(filename, model)) {
            model.transformations = transformations;
            models.push_back(model);
            std::cout << "Successfully loaded model from " << filename << std::endl;
        } else {
            std::cerr << "Failed to load model from " << filename << std::endl;
        }
    }

    for (const auto& childGroup : group.childGroups) {
        loadModelsGroup(childGroup, transformations);
    }
}

void loadModels() {
    for(const auto& group : world.groups){
        loadModelsGroup(group, {});
    }
}

void run_engine(World new_world, int argc, char **argv) {
	world = new_world;

    initializeCamera();

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
    glutMotionFunc(processMouseMotion);
    glutMouseFunc(processMouseButtons);

    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_LINE);

    updateCamera();

    // enter GLUT's main cycle
    glutMainLoop();

}