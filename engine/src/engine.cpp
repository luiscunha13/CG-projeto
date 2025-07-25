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

World world = {};
std::vector<Model> models;
std::vector<Model> skyboxes;

bool cameraChanged = true;
bool thirdpersoncamera = false;

float camX, camY, camZ;
float lookAtX, lookAtY, lookAtZ;
float upX, upY, upZ;

float hAngle = 0.0f;   // Horizontal angle (left/right)
float vAngle = 0.0f; // Vertical angle (up/down)
float speed = 1.0f; // Movement speed

float fov = 45.0f;

int targetModelIndex = -1;

bool mouseLeftDown = false;
int lastMouseX = -1, lastMouseY = -1;

bool cullingVisible = false;
bool axisvisible = false;
bool wireframeMode = false;

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

    float ratio = w * 1.0 / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, w, h);

    gluPerspective(fov ,ratio, 1.0f ,1000.0f);

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
        std::vector<Vertex3f> tangents = calculateTangents(points);
        return getGlobalHermitePoint(points, tangents, normalizedTime, derivOut);
    } else {
        return getGlobalCatmullRomPoint(points, normalizedTime, derivOut);
    }
}


void renderCatmullRomCurve(const std::vector<Vertex3f>& points) {

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);

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
    float amb[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

    const auto nrlights = std::min(world.lights.size(), (size_t)8);

    for (int i = 0; i < 8; ++i){
        glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, 180);
        glDisable(GL_LIGHT0 + i);
    }

    float w[4] = {1.0, 1.0, 1.0, 1.0};

    for (int i = 0; i < nrlights; ++i){
        GLenum lightID = GL_LIGHT0 + i;

        glEnable(lightID);
        glLightfv(lightID, GL_DIFFUSE, w);
        glLightfv(lightID, GL_SPECULAR, w);
    }


}

void renderLights(const World& world){
    const auto nrlights = std::min(world.lights.size(), (size_t)8);

    for (size_t i = 0; i < nrlights; i++) {
        GLenum lightID = GL_LIGHT0 + i;
        const Light& light = world.lights[i];

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
    std::ifstream fileCheck(s);
    if (!fileCheck) {
        std::cerr << "ERROR: Texture file does not exist: " << s << std::endl;
        return 0;
    }
    fileCheck.close();

    unsigned int t, tw, th;
    unsigned char *texData;
    unsigned int texID = 0;

    static bool ilInitialized = false;
    if (!ilInitialized) {
        ilInit();
        ilEnable(IL_ORIGIN_SET);
        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
        ilInitialized = true;
        std::cout << "DevIL initialized" << std::endl;
    }

    ilGenImages(1, &t);
    ilBindImage(t);

    ILboolean success = ilLoadImage((ILstring)s.c_str());
    if (!success) {
        ILenum error = ilGetError();
        std::cerr << "Failed to load image: " << s << std::endl;
        ilDeleteImages(1, &t);
        return 0;
    }

    success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    if (!success) {
        ILenum error = ilGetError();
        std::cerr << "Failed to convert image: " << s << std::endl;
        ilDeleteImages(1, &t);
        return 0;
    }

    tw = ilGetInteger(IL_IMAGE_WIDTH);
    th = ilGetInteger(IL_IMAGE_HEIGHT);
    texData = ilGetData();

    glGenTextures(1, &texID);
    if (texID == 0) {
        std::cerr << "Failed to generate GL texture ID" << std::endl;
        ilDeleteImages(1, &t);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glGenerateMipmap(GL_TEXTURE_2D);


    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR) {
        std::cerr << "OpenGL Error in texture creation: " << glError << std::endl;
    }

    ilDeleteImages(1, &t);

    return texID;
}

void renderModel(const Model& model) {
    if (model.isSkybox) {
        glDisable(GL_LIGHTING);

        if (model.hasTexture && model.textureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, model.textureID);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        }

        glDisable(GL_CULL_FACE);

        glDepthFunc(GL_LEQUAL);

        glBindBuffer(GL_ARRAY_BUFFER, model.vertexBuffer);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3f), 0);

        if (model.hasTexture && model.textureID > 0) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3f), (void*)offsetof(Vertex3f, s));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);
        glDrawElements(GL_TRIANGLES, model.n_indices, GL_UNSIGNED_INT, 0);

        glDisableClientState(GL_VERTEX_ARRAY);
        if (model.hasTexture && model.textureID > 0) {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
        glEnable(GL_LIGHTING);
        if (model.hasTexture && model.textureID > 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    } else {

        float diffuse[4] = {model.material.diffuse.x, model.material.diffuse.y, model.material.diffuse.z, 1.0f};
        float ambient[4] = {model.material.ambient.x, model.material.ambient.y, model.material.ambient.z, 1.0f};
        float specular[4] = {model.material.specular.x, model.material.specular.y, model.material.specular.z, 1.0f};
        float emissive[4] = {model.material.emissive.x, model.material.emissive.y, model.material.emissive.z, 1.0f};

        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
        glMaterialf(GL_FRONT, GL_SHININESS, model.material.shininess);

        if (model.hasTexture && model.textureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, model.textureID);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        } else {
            glDisable(GL_TEXTURE_2D);
        }

        glBindBuffer(GL_ARRAY_BUFFER, model.vertexBuffer);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3f), 0);

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, sizeof(Vertex3f), (void*)offsetof(Vertex3f, nx));

        if (model.hasTexture && model.textureID > 0) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3f), (void*)offsetof(Vertex3f, s));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);
        glDrawElements(GL_TRIANGLES, model.n_indices, GL_UNSIGNED_INT, 0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        if (model.hasTexture && model.textureID > 0) {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        if (model.hasTexture && model.textureID > 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
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

std::pair<Vertex3f, float> getTransformedSphere(const BoundingSphere& sphere,
                                                const std::vector<Transformation>& transformations,
                                                bool applyAnimation = true) {
    glPushMatrix();
    glLoadIdentity();

    applyTransformations(transformations, applyAnimation);

    float modelView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glPopMatrix();

    Vertex3f center = sphere.center;
    Vertex3f tCenter = {
            modelView[0] * center.x + modelView[4] * center.y + modelView[8] * center.z + modelView[12],
            modelView[1] * center.x + modelView[5] * center.y + modelView[9] * center.z + modelView[13],
            modelView[2] * center.x + modelView[6] * center.y + modelView[10] * center.z + modelView[14]
    };

    float scaleX = sqrt(modelView[0]*modelView[0] + modelView[1]*modelView[1] + modelView[2]*modelView[2]);
    float scaleY = sqrt(modelView[4]*modelView[4] + modelView[5]*modelView[5] + modelView[6]*modelView[6]);
    float scaleZ = sqrt(modelView[8]*modelView[8] + modelView[9]*modelView[9] + modelView[10]*modelView[10]);
    float maxScale = std::max({scaleX, scaleY, scaleZ});

    return {tCenter, sphere.radius * maxScale};
}

void updateCamera() {
    if (targetModelIndex >= 0 && targetModelIndex < models.size()) {
        const Model& target = models[targetModelIndex];

        auto [sphereCenter, sphereRadius] = getTransformedSphere(target.boundingSphere, target.transformations, true);

        float adaptiveFollowDistance = sphereRadius * 3.0f;

        float adaptiveFollowHeight = sphereRadius * 0.75f;

        camX = sphereCenter.x + adaptiveFollowDistance * cos(hAngle) * cos(vAngle);
        camY = sphereCenter.y + adaptiveFollowHeight + adaptiveFollowDistance * sin(vAngle);
        camZ = sphereCenter.z + adaptiveFollowDistance * sin(hAngle) * cos(vAngle);

        float lookAtHeight = sphereRadius * 0.5f;
        lookAtX = sphereCenter.x;
        lookAtY = sphereCenter.y + lookAtHeight;
        lookAtZ = sphereCenter.z;

        upX = 0.0f;
        upY = 1.0f;
        upZ = 0.0f;

    } else {
        lookAtX = camX + cos(hAngle) * cos(vAngle);
        lookAtY = camY + sin(vAngle);
        lookAtZ = camZ + sin(hAngle) * cos(vAngle);

        upX = -cos(hAngle) * sin(vAngle);
        upY = cos(vAngle);
        upZ = -sin(hAngle) * sin(vAngle);
    }

    cameraChanged = true;
}

void setThirdPersonCamera(int modelIndex) {
    targetModelIndex = modelIndex;

    hAngle = M_PI;
    vAngle = -0.3f;

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

    Vertex3f center = {0, 0, 0};
    for (const auto& v : model.vertices) {
        center.x += v.x;
        center.y += v.y;
        center.z += v.z;
    }
    center.x /= model.vertices.size();
    center.y /= model.vertices.size();
    center.z /= model.vertices.size();

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

void renderBoundingSphere(const Vertex3f& center, float radius) {
    glDisable(GL_LIGHTING);

    float color[3] = {0.0f, 1.0f, 0.0f};

    glColor3fv(color);

    glPushMatrix();

    glTranslatef(center.x, center.y, center.z);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glutWireSphere(radius, 16, 16);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glPopMatrix();

    glEnable(GL_LIGHTING);
}

void drawAxis(){
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

void printFPS(float fps) {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    std::string fpsText = "FPS: " + std::to_string(fps);
    fpsText = fpsText.substr(0, fpsText.find('.') + 3);

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 20);
    for (char c : fpsText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}

void renderScene() {

    static int frameCount = 0;
    static float fps = 0;
    static int currentTime = 0;
    static int previousTime = glutGet(GLUT_ELAPSED_TIME);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(camX,camY,camZ,
              lookAtX, lookAtY, lookAtZ,
              upX, upY, upZ);

    if (cameraChanged) {
        extractFrustumPlanes();
        cameraChanged = false;
    }

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

    int totalModels = models.size() + skyboxes.size();
    int renderedModels = 0;

    glDepthMask(GL_FALSE);
    for (auto& model : skyboxes) {
        glPushMatrix();

        glTranslatef(camX, camY, camZ);

        applyTransformations(model.transformations);

        if (!model.vboInitialized) {
            initModelVBOs(model);
        }
        renderModel(model);

        glPopMatrix();
        renderedModels++;
    }
    glDepthMask(GL_TRUE);

    renderLights(world);

    if(axisvisible)
        drawAxis();

    for (auto& model : models) {
        if (model.hasBoundingSphere) {
            auto [sphereCenter, sphereRadius] = getTransformedSphere(model.boundingSphere, model.transformations, true);

            if (!isSphereInFrustum(sphereCenter, sphereRadius)) {
                continue;
            }
            renderedModels++;

            if(cullingVisible){
                renderBoundingSphere(sphereCenter, sphereRadius);
            }

        }

        glPushMatrix();
        applyTransformations(model.transformations);

        if (!model.vboInitialized) {
            initModelVBOs(model);
        }
        renderModel(model);

        glPopMatrix();
    }

    frameCount++;
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    int timeInterval = currentTime - previousTime;

    if (timeInterval > 1000) {
        fps = frameCount / (timeInterval / 1000.0f);
        previousTime = currentTime;
        frameCount = 0;
    }

    printRenderedModels(totalModels, renderedModels);
    printFPS(fps);

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

    float dx = lookAtX - camX;
    float dy = lookAtY - camY;
    float dz = lookAtZ - camZ;

    hAngle = atan2(dz, dx);

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
            thirdpersoncamera = false;
            setFirstPersonCamera();
            break;
        case '3': // Switch to third-person camera (follow first model)
            if (!models.empty()) {
                thirdpersoncamera = true;
                setThirdPersonCamera(0);
            }
            break;
        case 'f':
            cullingVisible = !cullingVisible; // see bounding spheres
            std::cout << "Bounding Spheres Visible: " << (cullingVisible ? "Enabled" : "Disabled") << std::endl;
            break;
        case 'x':
            axisvisible = !axisvisible; // see axis
            std::cout << "Axis: " << (axisvisible ? "Visible" : "Hidden") << std::endl;
            break;
        case 'l':
            wireframeMode = !wireframeMode; // enable wireframe
            if (wireframeMode) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                std::cout << "Wireframe mode: ON" << std::endl;
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                std::cout << "Wireframe mode: OFF" << std::endl;
            }
            break;
    }
    updateCamera();
    glutPostRedisplay();
}

void processSpecialKeys(int key, int x, int y){
    switch (key){
        case GLUT_KEY_RIGHT:
            if(thirdpersoncamera){
                if (!models.empty()) {
                    if (targetModelIndex == -1) {
                        setThirdPersonCamera(0);
                    } else {
                        setThirdPersonCamera((targetModelIndex + 1) % models.size());
                    }
                }
            }
            break;
        case GLUT_KEY_LEFT:
            if(thirdpersoncamera){
                if (!models.empty()) {
                    if (targetModelIndex == -1) {
                        setThirdPersonCamera(models.size() - 1);
                    } else {
                        setThirdPersonCamera((targetModelIndex - 1 + models.size()) % models.size());
                    }
                }
            }
            break;
    }
}

void processMouseMotion(int x, int y) {
    if (!mouseLeftDown || lastMouseX == -1 || lastMouseY == -1) {
        return;
    }

    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;

    const float sensitivity = 0.002f;

    hAngle -= deltaX * sensitivity;
    vAngle += deltaY * sensitivity;

    const float maxVAngle = M_PI / 2.0f - 0.01f;
    if (vAngle > maxVAngle) vAngle = maxVAngle;
    if (vAngle < -maxVAngle) vAngle = -maxVAngle;

    lastMouseX = x;
    lastMouseY = y;

    updateCamera();
    glutPostRedisplay();
}

void processMouseButtons(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mouseLeftDown = false;
            lastMouseX = -1;
            lastMouseY = -1;
        }
    }

    if (state == GLUT_DOWN) {

        if (button == 3) {
            fov -= 2.0f;
            if (fov < 10.0f) fov = 10.0f;
        } else if (button == 4) {
            fov += 2.0f;
            if (fov > 90.0f) fov = 90.0f;
        }

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

    std::string formatLine;
    std::getline(file, formatLine);
    bool hasNormalsAndTextures = (formatLine.find("NT") != std::string::npos);

    int n_triangles;
    if (!(file >> model.n_vertices >> n_triangles)) {
        std::cerr << "Error reading vertex and triangle counts from " << filename << std::endl;
        file.close();
        return false;
    }

    model.n_indices = n_triangles * 3;
    model.vertices.resize(model.n_vertices);

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

    model.indices.resize(model.n_indices);
    for (int i = 0; i < model.n_indices; i++) {
        if (!(file >> model.indices[i])) {
            std::cerr << "Error reading index " << i << " from " << filename << std::endl;
            file.close();
            return false;
        }
    }

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
            if(model.isSkybox)
                skyboxes.push_back(model);
            else
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
    glutSpecialFunc(processSpecialKeys);
    glutMotionFunc(processMouseMotion);
    glutMouseFunc(processMouseButtons);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);

    setupLighting(world);
    loadModels();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    updateCamera();
    extractFrustumPlanes();
    atexit(cleanupVBOs);

    glutMainLoop();

}