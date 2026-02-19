#include "camera.h"

Camera::Camera(
    const Vec3f& e,
    const Vec3f& c, 
    const Vec3f& u,
    float fov_deg,
    float aspect_ratio,
    float nearp,
    float farp
) : eye(e), center(c), up(u), 
    fov(fov_deg), aspect(aspect_ratio), 
    near_plane(nearp), far_plane(farp)
{
    computeMatrices();
}

void Camera::computeMatrices() {
    // View (LookAt)
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up, z).normalize();
    Vec3f y = cross(z, x).normalize();

    ModelView = Matrix::identity();
    for (int i = 0; i < 3; i++) {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -eye[i];
    }

    // Projection (Perspective)
    Projection = Matrix::identity();
    float coeff = -1.f / (eye - center).norm();
    Projection[3][2] = coeff;

    // Viewport
    Viewport = Matrix::identity();
    Viewport[0][3] = 800 / 2.f;  // width
    Viewport[1][3] = 800 / 2.f;  // height
    Viewport[2][3] = 255.f / 2.f;
    Viewport[0][0] = 800 / 2.f;
    Viewport[1][1] = 800 / 2.f;
    Viewport[2][2] = 255.f / 2.f;
}

Matrix Camera::getViewProjection() const {
    return Viewport * Projection * ModelView;
}
void Camera::apply() const {
    // Вызывает lookat и projection
    lookat(eye, center, up);
    projection(-1.f / (eye - center).norm());
}

Vec3f Camera::viewDir(const Vec3f& worldPos) const {
    // Возвращает направление от точки к камере
    return (eye - worldPos).normalize();
}
