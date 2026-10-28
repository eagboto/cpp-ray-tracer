#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "camera.h"
#include "material.h"
#include "sphere.h"

// Builds a scene: a ground plane, three large feature spheres, and a field of
// small spheres with randomly chosen materials.
HittableList buildScene() {
    HittableList world;

    auto ground = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground));

    for (int a = -7; a < 7; ++a) {
        for (int b = -7; b < 7; ++b) {
            Point3 center(a + 0.9 * randomDouble(), 0.2, b + 0.9 * randomDouble());
            if ((center - Point3(4, 0.2, 0)).length() <= 0.9) continue;  // keep clear of the big spheres

            double choice = randomDouble();
            std::shared_ptr<Material> material;
            if (choice < 0.75) {
                material = std::make_shared<Lambertian>(Color::random() * Color::random());
            } else if (choice < 0.92) {
                material = std::make_shared<Metal>(Color::random(0.5, 1), randomDouble(0, 0.4));
            } else {
                material = std::make_shared<Dielectric>(1.5);
            }
            world.add(std::make_shared<Sphere>(center, 0.2, material));
        }
    }

    world.add(std::make_shared<Sphere>(Point3(0, 1, 0), 1.0, std::make_shared<Dielectric>(1.5)));
    world.add(std::make_shared<Sphere>(Point3(-4, 1, 0), 1.0, std::make_shared<Lambertian>(Color(0.4, 0.2, 0.1))));
    world.add(std::make_shared<Sphere>(Point3(4, 1, 0), 1.0, std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0)));
    return world;
}

void printUsage() {
    std::cout << "Usage: raytracer [options]\n"
              << "  --width N     image width in pixels (default 400)\n"
              << "  --samples N   rays per pixel (default 50)\n"
              << "  --depth N     max bounces per ray (default 20)\n"
              << "  --threads N   worker threads (default: all CPU cores)\n"
              << "  --out FILE    output file (default render.ppm)\n";
}

int main(int argc, char* argv[]) {
    Camera camera;
    int threads = std::max(1u, std::thread::hardware_concurrency());
    std::string outFile = "render.ppm";

    // Simple command-line argument parsing.
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        bool hasValue = i + 1 < argc;
        if (arg == "--help" || arg == "-h") { printUsage(); return 0; }
        else if (arg == "--width" && hasValue) camera.imageWidth = std::atoi(argv[++i]);
        else if (arg == "--samples" && hasValue) camera.samplesPerPixel = std::atoi(argv[++i]);
        else if (arg == "--depth" && hasValue) camera.maxDepth = std::atoi(argv[++i]);
        else if (arg == "--threads" && hasValue) threads = std::atoi(argv[++i]);
        else if (arg == "--out" && hasValue) outFile = argv[++i];
        else { std::cerr << "Unknown option: " << arg << "\n"; printUsage(); return 1; }
    }

    if (camera.imageWidth < 1 || camera.samplesPerPixel < 1 || camera.maxDepth < 1 || threads < 1) {
        std::cerr << "Width, samples, depth and threads must all be at least 1.\n";
        return 1;
    }

    HittableList world = buildScene();

    std::clog << "Rendering " << camera.imageWidth << " px wide, "
              << camera.samplesPerPixel << " samples/pixel, " << threads << " thread(s)\n";

    auto start = std::chrono::steady_clock::now();
    std::vector<Color> pixels = camera.render(world, threads);
    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();

    std::ofstream out(outFile);
    if (!out) {
        std::cerr << "Could not open " << outFile << " for writing.\n";
        return 1;
    }
    camera.writePPM(out, pixels);

    std::clog << "Done in " << seconds << " s. Saved " << outFile << "\n";
    return 0;
}
