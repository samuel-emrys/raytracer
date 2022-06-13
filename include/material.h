#pragma once
#include <utility>

#include "color.h"
#include "utility.h"

struct HitRecord;

class Material {
  public:
    virtual auto scatter(const Ray& rInputRay,
                         const HitRecord& rHitRecord,
                         Color& rAttenuation,
                         Ray& rScatteredRay) const -> bool
        = 0;
    virtual ~Material() = default;
};

class Lambertian : public Material {
  public:
    explicit Lambertian(Color rColor) : mAlbedo(std::move(rColor)){};

    auto scatter(const Ray& rInputRay,
                 const HitRecord& rHitRecord,
                 Color& rAttenuation,
                 Ray& rScatteredRay) const -> bool override;

  private:
    /**
     * @brief The faction of light that that the material reflects
     */
    Color mAlbedo;
};

class Metal : public Material {
  public:
    explicit Metal(Color rColor, double rFuzz) : mAlbedo(std::move(rColor)), mFuzz(rFuzz) {};

    auto scatter(const Ray& rInputRay,
                 const HitRecord& rHitRecord,
                 Color& rAttenuation,
                 Ray& rScatteredRay) const -> bool override;

  private:
    /**
     * @brief The faction of light that that the material reflects
     */
    Color mAlbedo;
    double mFuzz;
};

class Dialectric : public Material {
    public:
        explicit Dialectric(double rRefractionIndex) : mRefractionIndex(rRefractionIndex) {};

    auto scatter(const Ray& rInputRay,
                 const HitRecord& rHitRecord,
                 Color& rAttenuation,
                 Ray& rScatteredRay) const -> bool override;

  private:

    [[nodiscard]] auto reflectance(double rCosTheta, double rRefractionIndex) const -> double;
    double mRefractionIndex;

};
