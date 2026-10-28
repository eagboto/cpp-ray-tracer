#pragma once

#include <cmath>
#include "random.h"

// A 3D vector. Used for points in space, directions, and RGB colors.
class Vec3 {
public:
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator-() const { return Vec3(-x, -y, -z); }

    Vec3& operator+=(const Vec3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }

    Vec3& operator*=(double t) {
        x *= t; y *= t; z *= t;
        return *this;
    }

    double lengthSquared() const { return x * x + y * y + z * z; }
    double length() const { return std::sqrt(lengthSquared()); }

    // True if the vector is extremely close to zero in every direction.
    bool nearZero() const {
        const double eps = 1e-8;
        return std::fabs(x) < eps && std::fabs(y) < eps && std::fabs(z) < eps;
    }

    static Vec3 random() { return Vec3(randomDouble(), randomDouble(), randomDouble()); }
    static Vec3 random(double min, double max) {
        return Vec3(randomDouble(min, max), randomDouble(min, max), randomDouble(min, max));
    }
};

// Aliases make the code easier to read: a Point3 is a location, a Color is RGB.
using Point3 = Vec3;
using Color = Vec3;

inline Vec3 operator+(const Vec3& a, const Vec3& b) { return Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vec3 operator-(const Vec3& a, const Vec3& b) { return Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vec3 operator*(const Vec3& a, const Vec3& b) { return Vec3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline Vec3 operator*(double t, const Vec3& v) { return Vec3(t * v.x, t * v.y, t * v.z); }
inline Vec3 operator*(const Vec3& v, double t) { return t * v; }
inline Vec3 operator/(const Vec3& v, double t) { return (1.0 / t) * v; }

inline double dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x);
}

inline Vec3 unitVector(const Vec3& v) { return v / v.length(); }

// Random direction on the unit sphere (rejection sampling: pick points in a
// cube until one lands inside the sphere, then normalize it).
inline Vec3 randomUnitVector() {
    while (true) {
        Vec3 p = Vec3::random(-1, 1);
        double lenSq = p.lengthSquared();
        if (lenSq > 1e-160 && lenSq <= 1.0) {
            return p / std::sqrt(lenSq);
        }
    }
}

// Random point inside a flat disk of radius 1 — used for camera blur (depth of field).
inline Vec3 randomInUnitDisk() {
    while (true) {
        Vec3 p(randomDouble(-1, 1), randomDouble(-1, 1), 0);
        if (p.lengthSquared() < 1.0) return p;
    }
}

// Mirror reflection of v around surface normal n.
inline Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2.0 * dot(v, n) * n;
}

// Bend a ray as it passes into glass/water (Snell's law).
// etaRatio = refractive index we're leaving / index we're entering.
inline Vec3 refract(const Vec3& unitDir, const Vec3& n, double etaRatio) {
    double cosTheta = std::fmin(dot(-unitDir, n), 1.0);
    Vec3 perpendicular = etaRatio * (unitDir + cosTheta * n);
    Vec3 parallel = -std::sqrt(std::fabs(1.0 - perpendicular.lengthSquared())) * n;
    return perpendicular + parallel;
}
