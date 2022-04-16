#pragma once
#include <Eigen/Dense>

using Point3d = Eigen::Vector3d;
using Vector3d = Eigen::Vector3d;

class Ray {
  public:
    Ray() = default;
    Ray(Point3d  rOrigin, Vector3d  rDirection) : mOrigin(std::move(rOrigin)), mDirection(std::move(rDirection)){};
    auto origin() const -> Vector3d;
    auto direction() const -> Vector3d;
    auto at(double t) const -> Vector3d;

  private:
    Point3d mOrigin;
    Vector3d mDirection;
};
