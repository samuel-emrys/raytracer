#include "ray.h"

auto Ray::origin() const -> Vector3d {
    return mOrigin;
}

auto Ray::direction() const -> Vector3d {
    return mDirection;
}

auto Ray::at(double t) const -> Point3d {
    return origin() + t * direction();
}

auto reflect(const Vector3d& rRay, const Vector3d& rNormal) -> Vector3d {
    return rRay - 2 * rRay.dot(rNormal) * rNormal;
}

auto refract(const Vector3d& rRay, const Vector3d& rNormal, double rRefractionIndexRatio) -> Vector3d {
    auto vCosTheta = std::fmin((-rRay).dot(rNormal), 1.0);
    Vector3d vPerpendicular = rRefractionIndexRatio * (rRay + vCosTheta * rNormal);
    Vector3d vParallel = -std::sqrt(std::fabs(1.0 - std::pow(vPerpendicular.norm(), 2.0))) * rNormal;
    return vPerpendicular + vParallel;
}
