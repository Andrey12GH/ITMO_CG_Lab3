#pragma once
#include "geometry.h"
#include "own_gl.h"
#include "camera.h"

#pragma once
#include "geometry.h"
// #include "own_gl.h"  // временно убери

class Camera {
public:
    Vec3f eye;
    Vec3f center;
    Vec3f up;
    float fov;
    float aspect;
    float near_plane;
    float far_plane;

    Camera(
        const Vec3f& e,
        const Vec3f& c, 
        const Vec3f& u,
        float fov_deg = 90.0f,
        float aspect_ratio = 1.0f,
        float nearp = 0.1f,
        float farp = 100.0f
    );
    void computeMatrices();
    Matrix getViewProjection() const;
    void apply() const;
    Vec3f viewDir(const Vec3f& worldPos) const;
};
