#include <random>

class RNG {
public:
    RNG() : gen(rd()), dis(0.0f, 1.0f) {} // Initialize random generator and distribution

    float rand_float(float min, float max) {
        dis.param(std::uniform_real_distribution<float>::param_type(min, max)); // Update the range of the distribution
        return dis(gen);
    }

private:
    std::random_device rd;    // Used for random seed
    std::mt19937 gen;         // Mersenne Twister engine
    std::uniform_real_distribution<float> dis; // Uniform distribution
};