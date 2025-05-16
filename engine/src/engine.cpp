#include <GL/glew.h>
#include <engine.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <Model.h>
#include <Frustum.h>
#include <vector>
#include <string>
#include <IL/il.h>
#include <GL/glut.h>
#include <unordered_set>

World world = {};
std::vector<Model> models;

bool cameraChanged = true;
float lastCamX, lastCamY, lastCamZ;
float lastLookAtX, lastLookAtY, lastLookAtZ;
float lastUpX, lastUpY, lastUpZ;
float lastFov;

float camX, camY, camZ;
float lookAtX, lookAtY, lookAtZ;
float upX, upY, upZ;

float hAngle = 0.0f;   // Horizontal angle (left/right)
float vAngle = 0.0f; // Vertical angle (up/down)
float speed = 1.0f; // Movement speed

float fov = 45.0f;

int targetModelIndex = -1;  // Index of the model to follow (-1 for none)
float followDistance = 5.0f; // Distance from target
float followHeight = 2.0f; // Height above target

bool mouseLeftDown = false;
int lastMouseX = -1, lastMouseY = -1;

float catmull_matrix[4][4] = {{-0.5f,  1.5f, -1.5f,  0.5f},
                              { 1.0f, -2.5f,  2.0f, -0.5f},
                              {-0.5f,  0.0f,  0.5f,  0.0f},
                              { 0.0f,  1.0f,  0.0f,  0.0f}};

float hermite_matrix[4][4] = {{2.0f, -2.0f, 1.0f, 1.0f},
                              {-3.0f, 3.0f, -2.0f, -1.0f},
                              {0.0f, 0.0f, 1.0f, 0.0f},
                              {1.0f, 0.0f, 0.0f, 0.0f}};


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

    cameraChanged = true;
}

void buildRotMatrix(float *x, float *y, float *z, float *m) {

    m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
    m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
    m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}


void cross(float *a, float *b, float *res) {

    res[0] = a[1]*b[2] - a[2]*b[1];
    res[1] = a[2]*b[0] - a[0]*b[2];
    res[2] = a[0]*b[1] - a[1]*b[0];
}


void normalize(float *a) {

    float l = sqrt(a[0]*a[0] + a[1] * a[1] + a[2] * a[2]);
    a[0] = a[0]/l;
    a[1] = a[1]/l;
    a[2] = a[2]/l;
}

void multMatrixVector(float *m, float *v, float *res) {
    for (int j = 0; j < 4; ++j) {
        res[j] = 0;
        for (int k = 0; k < 4; ++k) {
            res[j] += v[k] * m[j * 4 + k];
        }
    }
}

std::vector<Vertex3f> calculateTangents(const std::vector<Vertex3f>& points) {
    int n = points.size();
    std::vector<Vertex3f> tangents(n);

    for (int i = 0; i < n; i++) {
        int prev = (i - 1 + n) % n;
        int next = (i + 1) % n;

        tangents[i].x = (points[next].x - points[prev].x) * 0.5f;
        tangents[i].y = (points[next].y - points[prev].y) * 0.5f;
        tangents[i].z = (points[next].z - points[prev].z) * 0.5f;
    }
    return tangents;
}

void getPoint(float t, float *p0, float *p1, float *p2, float *p3, float *pos, float *deriv, char algorithm) {
    float a[4][3];
    for (int i = 0; i < 3; i++) {
        //hermite -> p2/p3 = m0/m1
        float p[4] = {p0[i], p1[i], p2[i], p3[i]};
        float res[4];
        float *matrix;
        if(algorithm == 'H'){
            matrix = (float *)hermite_matrix;
        }
        else{
            matrix = (float *)catmull_matrix;
        }

        multMatrixVector(matrix, p, res);
        a[0][i] = res[0];
        a[1][i] = res[1];
        a[2][i] = res[2];
        a[3][i] = res[3];
    }

    // Compute pos = T * A and deriv = T' * A
    float t_vec[4] = {t*t*t, t*t, t, 1};
    float t_deriv[4] = {3*t*t, 2*t, 1, 0};
    for (int i = 0; i < 3; i++) {
        pos[i] = 0;
        deriv[i] = 0;

        for (int j = 0; j < 4; j++) {
            pos[i] += t_vec[j] * a[j][i];
            deriv[i] += t_deriv[j] * a[j][i];
        }
    }
}

Vertex3f getGlobalCatmullRomPoint(const std::vector<Vertex3f>& points, float gt, float* derivOut = nullptr) {
    int pointCount = points.size();
    if (pointCount < 4) {
        if (derivOut) {
            derivOut[0] = 0.0f;
            derivOut[1] = 0.0f;
            derivOut[2] = 0.0f;
        }
        return Vertex3f{0,0,0};
    }

    float t = gt * pointCount;
    int index = floor(t);
    t = t - index;

    // indices store the points
    int indices[4];
    indices[0] = (index + pointCount-1)%pointCount;
    indices[1] = (indices[0]+1)%pointCount;
    indices[2] = (indices[1]+1)%pointCount;
    indices[3] = (indices[2]+1)%pointCount;

    float p0[3] = {points[indices[0]].x, points[indices[0]].y, points[indices[0]].z};
    float p1[3] = {points[indices[1]].x, points[indices[1]].y, points[indices[1]].z};
    float p2[3] = {points[indices[2]].x, points[indices[2]].y, points[indices[2]].z};
    float p3[3] = {points[indices[3]].x, points[indices[3]].y, points[indices[3]].z};

    float pos[3], deriv[3];
    getPoint(t, p0, p1, p2, p3, pos, deriv, 'C');

    if (derivOut) {
        derivOut[0] = deriv[0];
        derivOut[1] = deriv[1];
        derivOut[2] = deriv[2];
    }

    return Vertex3f{pos[0], pos[1], pos[2]};
}

Vertex3f getGlobalHermitePoint(const std::vector<Vertex3f>& points, const std::vector<Vertex3f>& tangents, float gt, float* derivOut = nullptr) {
    int pointCount = points.size();
    if (pointCount < 2 || tangents.size() < pointCount) {
        if (derivOut) {
            derivOut[0] = 0.0f;
            derivOut[1] = 0.0f;
            derivOut[2] = 0.0f;
        }
        return Vertex3f{0,0,0};
    }

    float t = gt * pointCount;
    int index = floor(t);
    t = t - index;

    int indices[2];
    indices[0] = (index + pointCount-1)%pointCount;
    indices[1] = (indices[0]+1)%pointCount;

    float p0[3] = {points[indices[0]].x, points[indices[0]].y, points[indices[0]].z};
    float p1[3] = {points[indices[1]].x, points[indices[1]].y, points[indices[1]].z};
    float m0[3] = {tangents[indices[0]].x, tangents[indices[0]].y, tangents[indices[0]].z};
    float m1[3] = {tangents[indices[1]].x, tangents[indices[1]].y, tangents[indices[1]].z};

    float pos[3], deriv[3];
    getPoint(t, p0, p1, m0, m1, pos, deriv, 'H');

    if (derivOut) {
        derivOut[0] = deriv[0];
        derivOut[1] = deriv[1];
        derivOut[2] = deriv[2];
    }

    return Vertex3f{pos[0], pos[1], pos[2]};
}

Vertex3f getPointOnCurve(const std::vector<Vertex3f>& points, float normalizedTime, float* derivOut, const std::string& algorithm) {
    if (algorithm == "hermite") {
        // Calculate tangents
        std::vector<Vertex3f> tangents = calculateTangents(points);
        return getGlobalHermitePoint(points, tangents, normalizedTime, derivOut);
    } else {
        return getGlobalCatmullRomPoint(points, normalizedTime, derivOut);
    }
}


void renderCatmullRomCurve(const std::vector<Vertex3f>& points) {
    if (points.size() < 4) return;  // Need at least 4 points for Catmull-Rom

    glColor3f(1.0f, 1.0f, 1.0f);  // White color for the curve
    glBegin(GL_LINE_LOOP);

    // Draw 100 segments for smooth curve
    for (int i = 0; i <= 100; ++i) {
        float gt = (float)i / 100.0f;
        Vertex3f point = getGlobalCatmullRomPoint(points, gt);
        glVertex3f(point.x, point.y, point.z);
    }

    glEnd();
}

void renderHermiteCurve(const std::vector<Vertex3f>& points, const std::vector<Vertex3f>& tangents) {
    if (points.size() < 2 || tangents.size() < points.size()) return;

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i <= 100; ++i) {
        float gt = (float)i / 100.0f;
        Vertex3f point = getGlobalHermitePoint(points, tangents, gt);
        glVertex3f(point.x, point.y, point.z);
    }

    glEnd();
}

void renderCurve(const std::vector<Vertex3f>& points, const std::string& algorithm) {
    if (algorithm == "hermite") {
        std::vector<Vertex3f> tangents = calculateTangents(points);
        renderHermiteCurve(points, tangents);
    } else {
        // Default to Catmull-Rom
        renderCatmullRomCurve(points);
    }
}

void initModelVBOs(Model& model) {
    if (model.vboInitialized) return;

    // Generate and bind vertex buffer
    glGenBuffers(1, &model.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, model.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 model.vertices.size() * sizeof(Vertex3f),
                 model.vertices.data(),
                 GL_STATIC_DRAW);

    // Generate and bind index buffer
    glGenBuffers(1, &model.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 model.indices.size() * sizeof(unsigned int),
                 model.indices.data(),
                 GL_STATIC_DRAW);

    model.vboInitialized = true;
}

void cleanupVBOs() {
    for (auto& model : models) {
        if (model.vboInitialized) {
            glDeleteBuffers(1, &model.vertexBuffer);
            glDeleteBuffers(1, &model.indexBuffer);
            model.vboInitialized = false;
        }
    }
}

void setupLighting(const World& world) {
    if (world.lights.empty()) {
        glDisable(GL_LIGHTING);
        return;
    }

    glEnable(GL_RESCALE_NORMAL);

    float amb[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);


    glEnable(GL_LIGHTING);

    for (size_t i = 0; i < world.lights.size() && world.lights.size() < 8; i++) {
        GLenum lightID = GL_LIGHT0 + i;
        const Light& light = world.lights[i];

        float w[4] = {1.0f, 1.0f, 1.0f, 1.0f};

        glLightfv(lightID, GL_DIFFUSE, w);
        glLightfv(lightID, GL_SPECULAR, w);


        switch (light.type) {
            case Light::DIRECTIONAL: {
                float direction[4] = {light.direction.x, light.direction.y, light.direction.z, 0.0f};
                glLightfv(lightID, GL_POSITION, direction);
                break;
            }

            case Light::POINT: {
                float position[4] = {light.position.x, light.position.y, light.position.z, 1.0f};
                glLightfv(lightID, GL_POSITION, position);

                break;
            }

            case Light::SPOT: {
                float position[4] = {light.position.x, light.position.y, light.position.z, 1.0f};
                float direction[4] = {light.direction.x, light.direction.y, light.direction.z, 0.0f};

                glLightfv(lightID, GL_POSITION, position);
                glLightfv(lightID, GL_SPOT_DIRECTION, direction);
                glLightf(lightID, GL_SPOT_CUTOFF, light.cutoff);

                break;
            }
        }
        glEnable(lightID);
    }
}

GLuint loadTexture(const std::string& s) {
    // Basic file existence check
    std::ifstream fileCheck(s);
    if (!fileCheck) {
        std::cerr << "ERROR: Texture file does not exist: " << s << std::endl;
        return 0;
    }
    fileCheck.close();

    unsigned int t, tw, th;
    unsigned char *texData;
    unsigned int texID = 0;

    // Initialize DevIL if not already initialized
    static bool ilInitialized = false;
    if (!ilInitialized) {
        ilInit();
        ilEnable(IL_ORIGIN_SET);
        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
        ilInitialized = true;
        std::cout << "DevIL initialized" << std::endl;
    }

    // Generate an IL image ID
    ilGenImages(1, &t);
    ilBindImage(t);

    // Attempt to load image and report detailed errors
    ILboolean success = ilLoadImage((ILstring)s.c_str());
    if (!success) {
        ILenum error = ilGetError();
        std::cerr << "Failed to load image: " << s << std::endl;
        ilDeleteImages(1, &t);
        return 0;
    }

    // Convert the image to RGBA format
    success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    if (!success) {
        ILenum error = ilGetError();
        std::cerr << "Failed to convert image: " << s << std::endl;
        ilDeleteImages(1, &t);
        return 0;
    }

    // Get image dimensions and data
    tw = ilGetInteger(IL_IMAGE_WIDTH);
    th = ilGetInteger(IL_IMAGE_HEIGHT);
    texData = ilGetData();

    // Generate OpenGL texture
    glGenTextures(1, &texID);
    if (texID == 0) {
        std::cerr << "Failed to generate GL texture ID" << std::endl;
        ilDeleteImages(1, &t);
        return 0;
    }

    // Bind and set texture parameters
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // send texture data to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);


    // Check for OpenGL errors
    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR) {
        std::cerr << "OpenGL Error in texture creation: " << glError << std::endl;
    }

    // Clean up IL image
    ilDeleteImages(1, &t);

    return texID;
}

void renderModel(const Model& model) {
    // Set material properties
    float diffuse[4] = {model.material.diffuse.x, model.material.diffuse.y, model.material.diffuse.z, 1.0f};
    float ambient[4] = {model.material.ambient.x, model.material.ambient.y, model.material.ambient.z, 1.0f};
    float specular[4] = {model.material.specular.x, model.material.specular.y, model.material.specular.z, 1.0f};
    float emissive[4] = {model.material.emissive.x, model.material.emissive.y, model.material.emissive.z, 1.0f};

    if (model.isSkybox) {
        glDisable(GL_DEPTH_TEST);
        glCullFace(GL_FRONT);
        glDepthMask(GL_FALSE);

        glPushMatrix();
        glLoadIdentity();

        // Only keep rotation (remove translation) so skybox always surrounds camera
        gluLookAt(0, 0, 0,
                  lookAtX - camX, lookAtY - camY, lookAtZ - camZ,
                  upX, upY, upZ);

        float skyboxEmissive[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, skyboxEmissive);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, skyboxEmissive);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, skyboxEmissive);
    } else {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, model.material.shininess);
    }

    if (model.hasTexture && model.textureID > 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, model.textureID);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    // Bind the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, model.vertexBuffer);

    // Set up vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex3f), 0);

    // Set up normal arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex3f), (void*)offsetof(Vertex3f, nx));

    // Set up texture coordinate arrays if texture exists
    if (model.hasTexture && model.textureID > 0) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3f), (void*)offsetof(Vertex3f, s));
    }

    // Bind the index buffer and draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);
    glDrawElements(GL_TRIANGLES, model.n_indices, GL_UNSIGNED_INT, 0);

    // Cleanup
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    if (model.hasTexture && model.textureID > 0) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (model.hasTexture && model.textureID > 0) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (model.isSkybox) {
        glPopMatrix();  // Restore original matrix
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
        glDepthMask(GL_TRUE);

        // Reset material
        float defaultColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultColor);
    }
}

static void applyTransformations(const std::vector<Transformation>& transformations, bool applyAnimation = true) {
    float elapsedTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Get current time once

    for (const auto& transformation : transformations) {
        switch (transformation.type) {
            case Transformation::Type::Translate:
                if (transformation.animated && applyAnimation) {
                    float normalizedTime = fmod(elapsedTime / transformation.animation.time, 1.0f);
                    float deriv[3];
                    Vertex3f point = getPointOnCurve(
                            transformation.animation.points,
                            normalizedTime,
                            transformation.animation.align ? deriv : nullptr,
                            transformation.animation.algorithm);
                    glTranslatef(point.x, point.y, point.z);

                    if (transformation.animation.align) {
                        normalize(deriv);
                        float up[3] = {0, 1, 0};
                        float right[3];
                        cross(deriv, up, right);
                        normalize(right);
                        cross(right, deriv, up);
                        normalize(up);
                        float m[16];
                        buildRotMatrix(deriv, up, right, m);
                        glMultMatrixf(m);
                    }
                } else {
                    glTranslatef(transformation.coords.x,
                                 transformation.coords.y,
                                 transformation.coords.z);
                }
                break;

            case Transformation::Type::Rotate:
                if (transformation.animated && applyAnimation) {
                    float angle = fmod(360.0f * elapsedTime / transformation.animation.time, 360.0f);
                    glRotatef(angle,
                              transformation.coords.x,
                              transformation.coords.y,
                              transformation.coords.z);
                } else {
                    glRotatef(transformation.angle,
                              transformation.coords.x,
                              transformation.coords.y,
                              transformation.coords.z);
                }
                break;

            case Transformation::Type::Scale:
                glScalef(transformation.coords.x,
                         transformation.coords.y,
                         transformation.coords.z);
                break;
        }
    }
}

static Vertex3f getTransformedPosition(const std::vector<Transformation>& transformations) {
    glPushMatrix();
    glLoadIdentity();
    applyTransformations(transformations);

    float modelView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glPopMatrix();

    return Vertex3f{modelView[12], modelView[13], modelView[14]};
}

std::pair<Vertex3f, float> getTransformedSphere(const BoundingSphere& sphere,
                                                const std::vector<Transformation>& transformations,
                                                bool applyAnimation = true) {
    glPushMatrix();
    glLoadIdentity();

    // Apply all transformations with the same animation timing as the model
    applyTransformations(transformations, applyAnimation);

    // Get the full transformation matrix
    float modelView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glPopMatrix();

    // Transform center point
    Vertex3f center = sphere.center;
    Vertex3f tCenter = {
            modelView[0] * center.x + modelView[4] * center.y + modelView[8] * center.z + modelView[12],
            modelView[1] * center.x + modelView[5] * center.y + modelView[9] * center.z + modelView[13],
            modelView[2] * center.x + modelView[6] * center.y + modelView[10] * center.z + modelView[14]
    };

    // Calculate scale factor (using the maximum of all scale components)
    float scaleX = sqrt(modelView[0]*modelView[0] + modelView[1]*modelView[1] + modelView[2]*modelView[2]);
    float scaleY = sqrt(modelView[4]*modelView[4] + modelView[5]*modelView[5] + modelView[6]*modelView[6]);
    float scaleZ = sqrt(modelView[8]*modelView[8] + modelView[9]*modelView[9] + modelView[10]*modelView[10]);
    float maxScale = std::max({scaleX, scaleY, scaleZ});

    return {tCenter, sphere.radius * maxScale};
}

void updateCamera() {
    if (targetModelIndex >= 0 && targetModelIndex < models.size()) {
        const Model& target = models[targetModelIndex];

        // Get the fully transformed sphere with animations
        auto [sphereCenter, sphereRadius] = getTransformedSphere(target.boundingSphere, target.transformations, true);

        // Set follow distance to be slightly larger than the sphere radius
        followDistance = sphereRadius * 2.0f;

        // Adjust height proportionally to the sphere size
        followHeight = sphereRadius * 1.0f;

        // Calculate camera position based on angles and distance
        camX = sphereCenter.x - followDistance * cos(hAngle) * cos(vAngle);
        camY = sphereCenter.y + followHeight + followDistance * sin(vAngle) * 0.5f;
        camZ = sphereCenter.z - followDistance * sin(hAngle) * cos(vAngle);

        // Look at the center of the sphere
        lookAtX = sphereCenter.x;
        lookAtY = sphereCenter.y + followHeight * 0.5f;
        lookAtZ = sphereCenter.z;
    } else {
        // First-person camera behavior
        lookAtX = camX + cos(hAngle) * cos(vAngle);
        lookAtY = camY + sin(vAngle);
        lookAtZ = camZ + sin(hAngle) * cos(vAngle);
    }


    // Update up vector based on vertical angle
    upX = -cos(hAngle) * sin(vAngle);
    upY = cos(vAngle);
    upZ = -sin(hAngle) * sin(vAngle);

    cameraChanged = (camX != lastCamX || camY != lastCamY || camZ != lastCamZ ||
                     lookAtX != lastLookAtX || lookAtY != lastLookAtY || lookAtZ != lastLookAtZ ||
                     upX != lastUpX || upY != lastUpY || upZ != lastUpZ ||
                     fov != lastFov);

    if (cameraChanged) {
        lastCamX = camX;
        lastCamY = camY;
        lastCamZ = camZ;
        lastLookAtX = lookAtX;
        lastLookAtY = lookAtY;
        lastLookAtZ = lookAtZ;
        lastUpX = upX;
        lastUpY = upY;
        lastUpZ = upZ;
        lastFov = fov;
    }
}

void setThirdPersonCamera(int modelIndex) {

    hAngle = M_PI; // Face the front of the target
    vAngle = -0.3f; // Slightly look down

    updateCamera();
}

void setFirstPersonCamera() {
    targetModelIndex = -1;
    updateCamera();
}

void calculateBoundingVolumes(Model& model) {
    if (model.vertices.empty()) {
        model.hasBoundingSphere = false;
        return;
    }

    // Calculate center as the average of all vertices
    Vertex3f center = {0, 0, 0};
    for (const auto& v : model.vertices) {
        center.x += v.x;
        center.y += v.y;
        center.z += v.z;
    }
    center.x /= model.vertices.size();
    center.y /= model.vertices.size();
    center.z /= model.vertices.size();

    // Calculate maximum radius
    float maxRadius = 0.0f;
    for (const auto& v : model.vertices) {
        float dx = v.x - center.x;
        float dy = v.y - center.y;
        float dz = v.z - center.z;
        float distanceSquared = dx*dx + dy*dy + dz*dz;
        if (distanceSquared > maxRadius) {
            maxRadius = distanceSquared;
        }
    }

    model.boundingSphere.center = center;
    model.boundingSphere.radius = sqrt(maxRadius);
    model.hasBoundingSphere = true;
}

void renderBoundingSphere(const Vertex3f& center, float radius, const float color[3] = nullptr) {
    glDisable(GL_LIGHTING);

    // Set color (default to yellow if none specified)
    if (color) {
        glColor3fv(color);
    } else {
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    }

    // Save current matrix state
    glPushMatrix();

    // Move to sphere center
    glTranslatef(center.x, center.y, center.z);

    // Set up wireframe rendering
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Draw sphere (using GLUT's built-in sphere)
    glutWireSphere(radius, 16, 16);

    // Restore polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Restore matrix state
    glPopMatrix();

    glEnable(GL_LIGHTING);
}

void drawAxis(){
    //desenhar eixos
    glDisable(GL_LIGHTING);
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
    glEnd();

    glEnable(GL_LIGHTING);

    glColor3f(1.0f, 1.0f, 1.0f);
}

void printRenderedModels(int totalModels, int renderedModels){
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    std::string stats = "Models: " + std::to_string(renderedModels) + "/" + std::to_string(totalModels);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(10, 20);
    for (char c : stats) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}

void renderScene() {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(camX,camY,camZ,
              lookAtX, lookAtY, lookAtZ,
              upX, upY, upZ);

    if (cameraChanged) {
        extractFrustumPlanes();
        cameraChanged = false;
    }

    setupLighting(world);

    drawAxis();


    /*
    for (const auto& model : models) {
        for (const auto& transformation : model.transformations) {
            if (transformation.type == Transformation::Type::Translate &&
                transformation.animated &&
                transformation.animation.points.size() >= 4 &&
                transformation.animation.algorithm == "hermite") {
                renderCurve(transformation.animation.points, transformation.animation.algorithm);
            }
        }
    }
    */
    int totalModels = models.size();
    int renderedModels = 0;

    for (auto& model : models) {
        if (model.hasBoundingSphere) {
            auto [sphereCenter, sphereRadius] = getTransformedSphere(model.boundingSphere, model.transformations, true);

            if (!isSphereInFrustum(sphereCenter, sphereRadius)) {
                continue; // Skip this model if it's not visible
            }
            renderedModels++;

            float color[3] = {0.0f, 1.0f, 0.0f}; // Green for visible
            renderBoundingSphere(sphereCenter, sphereRadius, color);
        }

        // Render the model
        glPushMatrix();
        applyTransformations(model.transformations);

        if (!model.vboInitialized) {
            initModelVBOs(model);
        }
        renderModel(model);

        glPopMatrix();
    }

    printRenderedModels(totalModels, renderedModels);

    updateCamera();
    glutSwapBuffers();
    glutPostRedisplay();
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

    std::cout << "Initial Camera Position: ("
              << camX << ", " << camY << ", " << camZ << ")" << std::endl;
    std::cout << "Initial Look-At: ("
              << lookAtX << ", " << lookAtY << ", " << lookAtZ << ")" << std::endl;
    std::cout << "Initial Angles - Horizontal: " << hAngle
              << ", Vertical: " << vAngle << std::endl;
}

void processKeys(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': // Move forward
            if (targetModelIndex == -1) {
                camX += speed * cos(hAngle);
                camZ += speed * sin(hAngle);
            }
            break;
        case 's': // Move backward
            if (targetModelIndex == -1) {
                camX -= speed * cos(hAngle);
                camZ -= speed * sin(hAngle);
            }
            break;
        case 'a': // Move left
            if (targetModelIndex == -1) {
                camX += speed * sin(hAngle);
                camZ -= speed * cos(hAngle);
            }
            break;
        case 'd': // Move right
            if (targetModelIndex == -1) {
                camX -= speed * sin(hAngle);
                camZ += speed * cos(hAngle);
            }
            break;
        case 'q': // Move up
            if (targetModelIndex == -1) camY += speed;
            break;
        case 'e': // Move down
            if (targetModelIndex == -1) camY -= speed;
            break;
        case '+': // Increase movement speed
            speed *= 1.1f;
            std::cout << "Speed: " << speed << std::endl;
            break;
        case '-': // Decrease movement speed
            speed *= 0.9f;
            std::cout << "Speed: " << speed << std::endl;
            break;
        case '1': // Switch to first-person camera
            setFirstPersonCamera();
            break;
        case '3': // Switch to third-person camera (follow first model)
            if (!models.empty()) {
                setThirdPersonCamera(0);
            }
            break;
        case 'n': // Next model
            if (!models.empty()) {
                if (targetModelIndex == -1) {
                    // If not following any model, start with first one
                    setThirdPersonCamera(0);
                } else {
                    // Cycle to next model
                    setThirdPersonCamera((targetModelIndex + 1) % models.size());
                }
            }
            break;

        case 'p': // Previous model
            if (!models.empty()) {
                if (targetModelIndex == -1) {
                    // If not following any model, start with last one
                    setThirdPersonCamera(models.size() - 1);
                } else {
                    // Cycle to previous model
                    setThirdPersonCamera((targetModelIndex - 1 + models.size()) % models.size());
                }
            }
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
            if (fov < 10.0f) fov = 10.0f;
        } else if (button == 4) {  // Scroll down - zoom out
            fov += 2.0f;
            if (fov > 90.0f) fov = 90.0f;
        }

        // Update projection matrix and frustum
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        int width = glutGet(GLUT_WINDOW_WIDTH);
        int height = glutGet(GLUT_WINDOW_HEIGHT);
        float ratio = width * 1.0f / height;
        gluPerspective(fov, ratio, 0.1f, 1000.0f);
        glMatrixMode(GL_MODELVIEW);

        extractFrustumPlanes();
        cameraChanged = true;
    }
    updateCamera();
    glutPostRedisplay();
}

bool readModelFromFile(const std::string& filename, Model& model, const ModelInfo& modelInfo) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    // Read first line to determine format
    std::string formatLine;
    std::getline(file, formatLine);
    bool hasNormalsAndTextures = (formatLine.find("NT") != std::string::npos);

    // Read number of vertices and triangles
    int n_triangles;
    if (!(file >> model.n_vertices >> n_triangles)) {
        std::cerr << "Error reading vertex and triangle counts from " << filename << std::endl;
        file.close();
        return false;
    }

    model.n_indices = n_triangles * 3; // Convert triangles to indices
    model.vertices.resize(model.n_vertices);

    // Read vertices based on format
    if (hasNormalsAndTextures) {
        // Full format: x y z nx ny nz s t
        for (int i = 0; i < model.n_vertices; i++) {
            Vertex3f& v = model.vertices[i];
            if (!(file >> v.x >> v.y >> v.z >> v.nx >> v.ny >> v.nz >> v.s >> v.t)) {
                std::cerr << "Error reading vertex " << i << " from " << filename << std::endl;
                file.close();
                return false;
            }
        }
    } else {
        // Basic format: x y z
        for (int i = 0; i < model.n_vertices; i++) {
            Vertex3f& v = model.vertices[i];
            if (!(file >> v.x >> v.y >> v.z)) {
                std::cerr << "Error reading vertex " << i << " from " << filename << std::endl;
                file.close();
                return false;
            }
        }
    }

    // Read indices
    model.indices.resize(model.n_indices);
    for (int i = 0; i < model.n_indices; i++) {
        if (!(file >> model.indices[i])) {
            std::cerr << "Error reading index " << i << " from " << filename << std::endl;
            file.close();
            return false;
        }
    }

    // Set material and texture
    model.material = modelInfo.material;
    if (!modelInfo.texture.empty()) {
        model.textureID = loadTexture(modelInfo.texture);
        model.hasTexture = (model.textureID != 0);
    } else {
        model.hasTexture = false;
    }

    calculateBoundingVolumes(model);

    model.vboInitialized = false;
    model.isSkybox = modelInfo.isSkybox;
    file.close();
    return true;
}

void loadModelsGroup(const Group& group, const std::vector<Transformation>& parentTransformations) {
    std::vector<Transformation> transformations = parentTransformations;
    transformations.insert(transformations.end(), group.transformations.begin(), group.transformations.end());

    for (const auto& modelInfo : group.models) {
        Model model;
        if (readModelFromFile(modelInfo.file, model, modelInfo)) {
            model.transformations = transformations;
            models.push_back(model);
            std::cout << "Successfully loaded model from " << modelInfo.file << std::endl;
        } else {
            std::cerr << "Failed to load model from " << modelInfo.file << std::endl;
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

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800,800);
    glutCreateWindow("CG@DI-UM");

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        exit(1);
    }

    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processKeys);
    glutMotionFunc(processMouseMotion);
    glutMouseFunc(processMouseButtons);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);

    loadModels();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    updateCamera();
    atexit(cleanupVBOs);

    // enter GLUT's main cycle
    glutMainLoop();

}