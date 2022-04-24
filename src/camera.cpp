#include "camera.h"
#include "ray.h"
#include "utility.h"
#include <cstdlib>

Camera::Camera(Point3d rLookFrom,
               const Point3d& rLookAt,      // NOLINT
               const Vector3d& rViewUp,     // NOLINT
               double rVerticalFieldOfView, // NOLINT
               double rAspectRatio,
               double rAperture,
               double rFocusDistance) :
    mOrigin(std::move(rLookFrom)), mLensRadius(rAperture / 2.0) { // NOLINT
    auto vHeight = std::tan(rVerticalFieldOfView / 2.0);
    auto vViewportHeight = 2.0 * vHeight;
    auto vViewportWidth = rAspectRatio * vViewportHeight;

    mW = (rLookFrom - rLookAt).normalized();
    mU = rViewUp.cross(mW).normalized();
    mV = mW.cross(mU);

    mHorizontal = rFocusDistance * vViewportWidth * mU;
    mVertical = rFocusDistance * vViewportHeight * mV;
    mLowerLeftCorner = mOrigin - mHorizontal / 2.0 - mVertical / 2.0 - rFocusDistance * mW;
};

auto Camera::getRay(double u, double v) const -> Ray {
    Vector3d vRadius = mLensRadius * randomInUnitDisk();
    Vector3d vOffset = mU * vRadius.x() + mV * vRadius.y();
    return {mOrigin + vOffset, mLowerLeftCorner + u * mHorizontal + v * mVertical - mOrigin - vOffset};
};
