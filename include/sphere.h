#pragma once
#include <utility>

#include "hittable.h"
#include "ray.h"

class Sphere : public Hittable {
  public:
    Sphere() = default;
    Sphere(Point3d rCenter, double rRadius, std::shared_ptr<Material> rMaterial) :
        mCenter(std::move(rCenter)), mRadius(rRadius), mMaterial(std::move(rMaterial)){};
    auto hit(const Ray& rRay,
             double rRayScalingFactorMin,
             double rRayScalingFactorMax,
             HitRecord& rHitRecord) const -> bool override;

  private:
    Point3d mCenter;
    double mRadius{};
    std::shared_ptr<Material> mMaterial;
};
