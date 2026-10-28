#pragma once

#include <cmath>
#include "hittable.h"

class Sphere : public Hittable {
public:
    Sphere(const Point3& center, double radius, std::shared_ptr<Material> material)
        : center(center), radius(std::fmax(0.0, radius)), material(material) {}

    // Solves the quadratic for where the ray crosses the sphere's surface:
    // |origin + t*dir - center|^2 = radius^2
    bool hit(const Ray& ray, double tMin, double tMax, HitRecord& rec) const override {
        Vec3 oc = center - ray.origin;
        double a = ray.direction.lengthSquared();
        double h = dot(ray.direction, oc);          // simplified "b/2" form
        double c = oc.lengthSquared() - radius * radius;
        double discriminant = h * h - a * c;
        if (discriminant < 0) return false;          // ray misses

        double sqrtD = std::sqrt(discriminant);

        // Try the nearer intersection first, then the farther one.
        double root = (h - sqrtD) / a;
        if (root <= tMin || root >= tMax) {
            root = (h + sqrtD) / a;
            if (root <= tMin || root >= tMax) return false;
        }

        rec.t = root;
        rec.point = ray.at(root);
        Vec3 outwardNormal = (rec.point - center) / radius;
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material;
        return true;
    }

private:
    Point3 center;
    double radius;
    std::shared_ptr<Material> material;
};
