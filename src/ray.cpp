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
