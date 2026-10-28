#pragma once

#include <cmath>
#include "hittable.h"

// A material decides what happens to light when a ray hits a surface:
// how the ray bounces (scattered) and how much color it keeps (attenuation).
class Material {
public:
    virtual ~Material() = default;
    virtual bool scatter(const Ray& incoming, const HitRecord& rec,
                         Color& attenuation, Ray& scattered) const = 0;
};

// Matte surface: light scatters in random directions.
class Lambertian : public Material {
public:
    explicit Lambertian(const Color& albedo) : albedo(albedo) {}

    bool scatter(const Ray&, const HitRecord& rec,
                 Color& attenuation, Ray& scattered) const override {
        Vec3 direction = rec.normal + randomUnitVector();
        if (direction.nearZero()) direction = rec.normal;  // avoid a zero-length ray
        scattered = Ray(rec.point, direction);
        attenuation = albedo;
        return true;
    }

private:
    Color albedo;
};

// Shiny surface: mirror reflection, optionally blurred by "fuzz".
class Metal : public Material {
public:
    Metal(const Color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const Ray& incoming, const HitRecord& rec,
                 Color& attenuation, Ray& scattered) const override {
        Vec3 reflected = unitVector(reflect(incoming.direction, rec.normal));
        reflected = reflected + fuzz * randomUnitVector();
        scattered = Ray(rec.point, reflected);
        attenuation = albedo;
        return dot(scattered.direction, rec.normal) > 0;  // absorbed if it goes into the surface
    }

private:
    Color albedo;
    double fuzz;
};

// Glass/water: rays either refract (pass through and bend) or reflect.
class Dielectric : public Material {
public:
    explicit Dielectric(double refractiveIndex) : refractiveIndex(refractiveIndex) {}

    bool scatter(const Ray& incoming, const HitRecord& rec,
                 Color& attenuation, Ray& scattered) const override {
        attenuation = Color(1.0, 1.0, 1.0);  // clear glass absorbs nothing
        double ratio = rec.frontFace ? (1.0 / refractiveIndex) : refractiveIndex;

        Vec3 unitDir = unitVector(incoming.direction);
        double cosTheta = std::fmin(dot(-unitDir, rec.normal), 1.0);
        double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

        // Total internal reflection: at steep angles light can't escape the glass.
        bool cannotRefract = ratio * sinTheta > 1.0;

        Vec3 direction;
        if (cannotRefract || reflectance(cosTheta, ratio) > randomDouble()) {
            direction = reflect(unitDir, rec.normal);
        } else {
            direction = refract(unitDir, rec.normal, ratio);
        }
        scattered = Ray(rec.point, direction);
        return true;
    }

private:
    double refractiveIndex;

    // Schlick's approximation: glass looks more mirror-like at glancing angles.
    static double reflectance(double cosine, double ratio) {
        double r0 = (1 - ratio) / (1 + ratio);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow(1 - cosine, 5);
    }
};
