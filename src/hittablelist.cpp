#include "hittablelist.h"

HittableList::HittableList(const std::shared_ptr<Hittable>& rObject) {
    add(rObject);
};

auto HittableList::clear() -> void {
    mObjects.clear();
};
auto HittableList::add(const std::shared_ptr<Hittable>& rObject) -> void {
    mObjects.push_back(rObject);
};
auto HittableList::hit(const Ray& rRay,
                       double rRayScalingFactorMin, // NOLINT
                       double rRayScalingFactorMax,
                       HitRecord& rHitRecord) const -> bool { 
    HitRecord vHitRecord;
    bool vHitAnything = false;
    auto vClosestSoFar = rRayScalingFactorMax;

    for (const auto& vObject : mObjects) {
        if (vObject->hit(rRay, rRayScalingFactorMin, vClosestSoFar, vHitRecord)) {
            vHitAnything = true;
            vClosestSoFar = vHitRecord.rayScalingFactor;
            rHitRecord = vHitRecord;
        }
    }

    return vHitAnything;
};
