#include "material.h"
#include "hittable.h"
#include "ray.h"
#include "utility.h"

auto Lambertian::scatter(const Ray& /*rInputRay*/, // Unused param
                         const HitRecord& rHitRecord,
                         Color& rAttenuation,
                         Ray& rScatteredRay) const -> bool {
    Vector3d vScatterDirection = rHitRecord.normal + randomUnitVector();

    if (vectorNearZero(vScatterDirection)) {
        vScatterDirection = rHitRecord.normal;
    }

    rScatteredRay = Ray(rHitRecord.point, vScatterDirection);
    rAttenuation = mAlbedo;
    return true;
};

auto Metal::scatter(const Ray& rInputRay,
                    const HitRecord& rHitRecord,
                    Color& rAttenuation,
                    Ray& rScatteredRay) const -> bool {
    Vector3d vReflected = reflect(rInputRay.direction().normalized(), rHitRecord.normal);
    rScatteredRay = Ray(rHitRecord.point, vReflected + mFuzz * randomInUnitSphere());
    rAttenuation = mAlbedo;
    return (rScatteredRay.direction().dot(rHitRecord.normal) > 0);
};

auto Dialectric::scatter(const Ray& rInputRay,
                         const HitRecord& rHitRecord,
                         Color& rAttenuation,
                         Ray& rScatteredRay) const -> bool {
    rAttenuation = Color(1.0, 1.0, 1.0);
    double vRefractionRatio
        = rHitRecord.frontFace ? (1.0 / mRefractionIndex) : mRefractionIndex;
    Vector3d vUnitDirection = rInputRay.direction().normalized();
    double vCosTheta = std::fmin((-vUnitDirection).dot(rHitRecord.normal), 1.0);
    double vSinTheta = std::sqrt(1.0 - std::pow(vCosTheta, 2.0));
    bool vCannotRefract = vRefractionRatio * vSinTheta > 1.0;
    Vector3d vDirection;

    if (vCannotRefract
        || reflectance(vCosTheta, vRefractionRatio) > randomNumber<double>()) {
        vDirection = reflect(vUnitDirection, rHitRecord.normal);
    } else {
        vDirection = refract(vUnitDirection, rHitRecord.normal, vRefractionRatio);
    }
    rScatteredRay = Ray(rHitRecord.point, vDirection);
    return true;
};

auto Dialectric::reflectance(double rCosTheta, double rRefractionIndex) const // NOLINT
    -> double {
    // Use Schlick's approximation for reflectance
    auto vR0 = std::pow((1 - rRefractionIndex) / (1 + rRefractionIndex), 2.0);
    return vR0 + (1 - vR0) * std::pow((1 - rCosTheta), 5.0);
}
