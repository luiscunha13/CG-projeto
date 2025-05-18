#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <cmath>
#include <array>
#include <GL/glut.h>
#include <vertex.h>

struct Plane {
    float a, b, c, d;

    void normalize() {
        float mag = sqrt(a*a + b*b + c*c);
        a /= mag;
        b /= mag;
        c /= mag;
        d /= mag;
    }

    float distance(const Vertex3f& point) const {
        return a * point.x + b * point.y + c * point.z + d;
    }
};

struct Frustum {
    Plane planes[6];
};

Frustum frustum;

void extractFrustumPlanes() {
    float proj[16];
    float modl[16];
    float clip[16];

    glGetFloatv(GL_PROJECTION_MATRIX, proj);
    glGetFloatv(GL_MODELVIEW_MATRIX, modl);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            clip[i*4+j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                clip[i*4+j] += modl[i*4+k] * proj[k*4+j];
            }
        }
    }

    // Right plane
    frustum.planes[0].a = clip[3] - clip[0];
    frustum.planes[0].b = clip[7] - clip[4];
    frustum.planes[0].c = clip[11] - clip[8];
    frustum.planes[0].d = clip[15] - clip[12];
    frustum.planes[0].normalize();

    // Left plane
    frustum.planes[1].a = clip[3] + clip[0];
    frustum.planes[1].b = clip[7] + clip[4];
    frustum.planes[1].c = clip[11] + clip[8];
    frustum.planes[1].d = clip[15] + clip[12];
    frustum.planes[1].normalize();

    // Bottom plane
    frustum.planes[2].a = clip[3] + clip[1];
    frustum.planes[2].b = clip[7] + clip[5];
    frustum.planes[2].c = clip[11] + clip[9];
    frustum.planes[2].d = clip[15] + clip[13];
    frustum.planes[2].normalize();

    // Top plane
    frustum.planes[3].a = clip[3] - clip[1];
    frustum.planes[3].b = clip[7] - clip[5];
    frustum.planes[3].c = clip[11] - clip[9];
    frustum.planes[3].d = clip[15] - clip[13];
    frustum.planes[3].normalize();

    // Far plane
    frustum.planes[4].a = clip[3] - clip[2];
    frustum.planes[4].b = clip[7] - clip[6];
    frustum.planes[4].c = clip[11] - clip[10];
    frustum.planes[4].d = clip[15] - clip[14];
    frustum.planes[4].normalize();

    // Near plane
    frustum.planes[5].a = clip[3] + clip[2];
    frustum.planes[5].b = clip[7] + clip[6];
    frustum.planes[5].c = clip[11] + clip[10];
    frustum.planes[5].d = clip[15] + clip[14];
    frustum.planes[5].normalize();
}

bool isSphereInFrustum(const Vertex3f& center, float radius) {
    for (int i = 0; i < 6; i++) {
        const Plane& p = frustum.planes[i];
        float distance = p.distance(center);
        if (distance < -radius) {
            return false;
        }
    }
    return true;
}

#endif