#include "camera.h"

Camera::Camera() {
    auto vAspectRatio = 16.0 / 9.0;
    auto vViewportHeight = 2.0;
    auto vViewportWidth = vAspectRatio * vViewportHeight;
    auto vFocalLength = 1.0;

    mOrigin = Point3d(0, 0, 0);
    mHorizontal = Vector3d(vViewportWidth, 0, 0);
    mVertical = Vector3d(0, vViewportHeight, 0);
    mLowerLeftCorner = mOrigin - mHorizontal / 2.0 - mVertical / 2.0 - Vector3d(0, 0, vFocalLength);
};

auto Camera::getRay(double u, double v) const -> Ray {
    return {mOrigin, mLowerLeftCorner + u * mHorizontal + v * mVertical - mOrigin};
};
