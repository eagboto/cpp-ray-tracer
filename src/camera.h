#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iostream>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>

#include "hittable.h"
#include "material.h"

class Camera {
public:
    // ---- Settings (set these before calling render) ----
    double aspectRatio = 16.0 / 9.0;
    int imageWidth = 400;
    int samplesPerPixel = 50;   // rays per pixel; more = smoother, slower
    int maxDepth = 20;          // max bounces per ray

    double verticalFov = 20;    // field of view in degrees
    Point3 lookFrom = Point3(13, 2, 3);
    Point3 lookAt = Point3(0, 0, 0);
    Vec3 viewUp = Vec3(0, 1, 0);

    double defocusAngle = 0.6;  // aperture size in degrees (0 = everything sharp)
    double focusDistance = 10;  // distance to the plane that's in focus

    int imageHeight() const { return imageHeight_; }

    // Renders the scene using `threadCount` threads. Returns one Color per pixel.
    std::vector<Color> render(const Hittable& world, int threadCount) {
        initialize();
        std::vector<Color> pixels(static_cast<size_t>(imageWidth) * imageHeight_);

        std::atomic<int> nextRow{0};
        std::atomic<int> rowsDone{0};
        std::mutex printMutex;

        // Each worker repeatedly grabs the next unrendered row until none are left.
        // Rows take different amounts of time, so this balances work better than
        // splitting the image into fixed chunks.
        auto worker = [&]() {
            while (true) {
                int j = nextRow.fetch_add(1);
                if (j >= imageHeight_) break;

                for (int i = 0; i < imageWidth; ++i) {
                    Color color(0, 0, 0);
                    for (int s = 0; s < samplesPerPixel; ++s) {
                        color += rayColor(getRay(i, j), maxDepth, world);
                    }
                    pixels[static_cast<size_t>(j) * imageWidth + i] = color / samplesPerPixel;
                }

                int done = ++rowsDone;
                if (done % 10 == 0 || done == imageHeight_) {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::clog << "\rRendering: " << (100 * done / imageHeight_) << "%   " << std::flush;
                }
            }
        };

        std::vector<std::thread> threads;
        for (int t = 0; t < threadCount; ++t) threads.emplace_back(worker);
        for (auto& thread : threads) thread.join();
        std::clog << "\n";
        return pixels;
    }

    // Writes pixels as a PPM image (a simple text image format).
    void writePPM(std::ostream& out, const std::vector<Color>& pixels) const {
        out << "P3\n" << imageWidth << ' ' << imageHeight_ << "\n255\n";
        for (const Color& c : pixels) {
            out << toByte(c.x) << ' ' << toByte(c.y) << ' ' << toByte(c.z) << '\n';
        }
    }

private:
    int imageHeight_ = 0;
    Point3 center;
    Point3 pixel00;
    Vec3 pixelDeltaU, pixelDeltaV;
    Vec3 u, v, w;
    Vec3 defocusDiskU, defocusDiskV;

    void initialize() {
        imageHeight_ = std::max(1, static_cast<int>(imageWidth / aspectRatio));
        center = lookFrom;

        // Size of the "window" in front of the camera that rays pass through.
        const double pi = 3.14159265358979323846;
        double theta = verticalFov * pi / 180.0;
        double viewportHeight = 2.0 * std::tan(theta / 2) * focusDistance;
        double viewportWidth = viewportHeight * (static_cast<double>(imageWidth) / imageHeight_);

        // Camera's own coordinate axes: w points backward, u right, v up.
        w = unitVector(lookFrom - lookAt);
        u = unitVector(cross(viewUp, w));
        v = cross(w, u);

        Vec3 viewportU = viewportWidth * u;
        Vec3 viewportV = viewportHeight * -v;   // image rows go downward
        pixelDeltaU = viewportU / imageWidth;
        pixelDeltaV = viewportV / imageHeight_;

        Point3 upperLeft = center - focusDistance * w - viewportU / 2 - viewportV / 2;
        pixel00 = upperLeft + 0.5 * (pixelDeltaU + pixelDeltaV);

        double defocusRadius = focusDistance * std::tan(defocusAngle * pi / 180.0 / 2);
        defocusDiskU = defocusRadius * u;
        defocusDiskV = defocusRadius * v;
    }

    // A ray through a random spot inside pixel (i, j). Random spots = anti-aliasing.
    Ray getRay(int i, int j) const {
        double offsetX = randomDouble() - 0.5;
        double offsetY = randomDouble() - 0.5;
        Point3 sample = pixel00 + (i + offsetX) * pixelDeltaU + (j + offsetY) * pixelDeltaV;

        Point3 origin = center;
        if (defocusAngle > 0) {
            Vec3 p = randomInUnitDisk();
            origin = center + p.x * defocusDiskU + p.y * defocusDiskV;
        }
        return Ray(origin, sample - origin);
    }

    // Follows a ray as it bounces around the scene and returns the color it sees.
    Color rayColor(const Ray& ray, int depth, const Hittable& world) const {
        if (depth <= 0) return Color(0, 0, 0);  // too many bounces: no light

        HitRecord rec;
        // tMin = 0.001 avoids "shadow acne" from a ray re-hitting its own surface.
        if (world.hit(ray, 0.001, std::numeric_limits<double>::infinity(), rec)) {
            Ray scattered;
            Color attenuation;
            if (rec.material->scatter(ray, rec, attenuation, scattered)) {
                return attenuation * rayColor(scattered, depth - 1, world);
            }
            return Color(0, 0, 0);
        }

        // Missed everything: return a sky gradient from white to blue.
        Vec3 dir = unitVector(ray.direction);
        double a = 0.5 * (dir.y + 1.0);
        return (1.0 - a) * Color(1.0, 1.0, 1.0) + a * Color(0.5, 0.7, 1.0);
    }

    // Gamma-correct (square root) and convert a 0–1 value to a 0–255 byte.
    static int toByte(double value) {
        value = value > 0 ? std::sqrt(value) : 0;
        value = std::clamp(value, 0.0, 0.999);
        return static_cast<int>(256 * value);
    }
};
