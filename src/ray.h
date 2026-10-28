#pragma once

#include "vec3.h"

// A ray is a starting point plus a direction: P(t) = origin + t * direction.
class Ray {
public:
    Point3 origin;
    Vec3 direction;

    Ray() = default;
    Ray(const Point3& origin, const Vec3& direction) : origin(origin), direction(direction) {}

    Point3 at(double t) const { return origin + t * direction; }
};
