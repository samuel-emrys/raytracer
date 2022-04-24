#pragma once
#include "ray.h"
#include <memory>

class Material;

struct HitRecord {
    Point3d point;
    Vector3d normal;
    std::shared_ptr<Material> material;
    double rayScalingFactor;
    bool frontFace;

    auto setFaceNormal(const Ray& rRay, const Vector3d& rOutwardNormal) -> void;
};

class Hittable {
    public:
        Hittable() = default;
        Hittable(const Hittable&) = default;
        Hittable(Hittable&&) = default;
        auto operator=(const Hittable&) -> Hittable& = default;
        auto operator=(Hittable&&) -> Hittable& = default;
        virtual ~Hittable() = default;
        virtual auto hit(const Ray& rRay, double rRayScalingFactorMin, double rRayScalingFactorMax, HitRecord& rHitRecord) const -> bool = 0;

};
