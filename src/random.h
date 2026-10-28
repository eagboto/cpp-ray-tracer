#pragma once

#include <random>

// Returns a random double in [0, 1).
// Each thread gets its own generator (thread_local) so threads never
// fight over shared random state — important for multithreaded rendering.
inline double randomDouble() {
    thread_local std::mt19937 generator(std::random_device{}());
    thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}

// Returns a random double in [min, max).
inline double randomDouble(double min, double max) {
    return min + (max - min) * randomDouble();
}
