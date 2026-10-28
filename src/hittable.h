#pragma once

#include <memory>
#include <vector>
#include "ray.h"

class Material;

// Everything we learn when a ray hits a surface.
struct HitRecord {
    Point3 point;
    Vec3 normal;                         // always points against the incoming ray
    std::shared_ptr<Material> material;
    double t = 0;                        // distance along the ray
    bool frontFace = true;               // true if the ray hit the outside

    void setFaceNormal(const Ray& ray, const Vec3& outwardNormal) {
        frontFace = dot(ray.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

// Abstract base class: anything a ray can hit.
class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& ray, double tMin, double tMax, HitRecord& rec) const = 0;
};

// A collection of objects. Finds the closest hit among all of them.
class HittableList : public Hittable {
public:
    std::vector<std::shared_ptr<Hittable>> objects;

    void add(std::shared_ptr<Hittable> object) { objects.push_back(object); }

    bool hit(const Ray& ray, double tMin, double tMax, HitRecord& rec) const override {
        HitRecord temp;
        bool hitAnything = false;
        double closest = tMax;

        for (const auto& object : objects) {
            if (object->hit(ray, tMin, closest, temp)) {
                hitAnything = true;
                closest = temp.t;   // only accept hits closer than this from now on
                rec = temp;
            }
        }
        return hitAnything;
    }
};
