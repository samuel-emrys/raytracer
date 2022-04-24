#pragma once

#include "ray.h"
#include "utility.h"

class Camera {
  public:
    Camera(Point3d rLookFrom,
           const Point3d& rLookAt,
           const Vector3d& rViewUp,
           double rVerticalFieldOfView,
           double rAspectRatio,
           double rAperture,
           double rFocusDistance);
    auto getRay(double u, double v) const -> Ray;

  private:
    Point3d mOrigin;
    Point3d mLowerLeftCorner;
    Vector3d mHorizontal;
    Vector3d mVertical;
    Vector3d mW;
    Vector3d mU;
    Vector3d mV;
    double mLensRadius;
};
