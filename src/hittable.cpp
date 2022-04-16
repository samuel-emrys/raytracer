#include "hittable.h"

auto HitRecord::setFaceNormal(const Ray& rRay, const Vector3d& rOutwardNormal) -> void {
    frontFace = rRay.direction().dot(rOutwardNormal) < 0;
    normal = frontFace ? rOutwardNormal : -1.0 * rOutwardNormal;
}
