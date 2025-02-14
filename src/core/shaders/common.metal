#include <metal_stdlib>
using namespace metal;

struct Parameters {
    float dx, dy, dz;
    float dx2, dy2, dz2;
    float dt;
    uint32_t nx, ny, nz;
    float current_time;
};

// Déclaration de la fonction force qui sera injectée
METAL_FUNC float f(float x, float y, float z, float t);
METAL_FUNC float g(float x, float y, float z);