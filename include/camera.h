#pragma once

#include "ray.h"
#include "utility.h"

class Camera {
  public:
    Camera();
    auto getRay(double u, double v) const -> Ray;

  private:
    Point3d mOrigin;
    Point3d mLowerLeftCorner;
    Vector3d mHorizontal;
    Vector3d mVertical;
};
