#include "sphere.h"
#include "ray.h"

auto Sphere::hit(const Ray& rRay, double rRayScalingFactorMin, double rRayScalingFactorMax, HitRecord& rHitRecord) const -> bool { // NOLINT
    Vector3d vSphereCenter = rRay.origin() - mCenter;
    // Use quadratic equation to find roots of intersection
    auto a = std::pow(rRay.direction().norm(), 2.0); // NOLINT [readability-identifier-naming]
    auto vHalfB = vSphereCenter.dot(rRay.direction());
    auto c = std::pow(vSphereCenter.norm(), 2.0) - std::pow(mRadius, 2.0); // NOLINT [readability-identifier-naming]

    auto vDiscriminant = std::pow(vHalfB, 2.0) - a * c;
    if (vDiscriminant < 0) {
        return false;
    }
    auto vSqrtDiscriminant = std::sqrt(vDiscriminant);

    // Find the nearest root that lies in the acceptable range
    auto vRoot = (-vHalfB - vSqrtDiscriminant) / a;
    if (vRoot < rRayScalingFactorMin || rRayScalingFactorMax < vRoot) {
        vRoot = (-vHalfB + vSqrtDiscriminant) / a;
        if (vRoot < rRayScalingFactorMin || rRayScalingFactorMax < vRoot) {
            return false;
        }
    }
    rHitRecord.rayScalingFactor = vRoot;
    rHitRecord.point = rRay.at(vRoot);
    Vector3d vOutwardNormal = (rHitRecord.point - mCenter) / mRadius;
    rHitRecord.setFaceNormal(rRay, vOutwardNormal);

    return true;
};
