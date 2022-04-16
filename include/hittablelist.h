#pragma once
#include "hittable.h"
#include <memory>
#include <vector>

class HittableList : public Hittable {
    public:
        HittableList() = default;
        explicit HittableList(const std::shared_ptr<Hittable>& rObject);

        auto clear() -> void;
        auto add(const std::shared_ptr<Hittable>& rObject) -> void;
        auto hit(const Ray& rRay, double rRayScalingFactorMin, double rRayScalingFactorMax, HitRecord& rHitRecord) const -> bool override;

    private:
        std::vector<std::shared_ptr<Hittable>> mObjects;
};
