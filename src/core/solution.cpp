/**
 * @file solution.cpp
 * @brief Implementation of the Solution class methods
 * @author Etienne Rosin
 * @date February 13, 2025
 * 
 * This file contains the implementation of the Solution class methods
 * defined in solution.hpp. It provides the concrete implementations for
 * grid initialization, access, and manipulation operations.
 */

#include "solution.hpp"

/**
 * @brief Constructor implementation
 * 
 * Initializes all grid parameters from the provided Parameters object
 * and allocates memory for the solution data array. The constructor
 * uses initialization lists for efficiency and const correctness.
 */
Solution::Solution(Parameters &params) :
    params(params),
    nx(params.getNx()), ny(params.getNy()), nz(params.getNz()),
    dx(params.getDx()), dx2(params.getDx2()),
    dy(params.getDy()), dy2(params.getDy2()),
    dz(params.getDz()), dz2(params.getDz2()) {
    data.resize(params.getNtot());
}

/**
 * @brief Implementation of non-const grid access operator
 * 
 * Converts 3D indices to 1D array index using the formula:
 * index = i + nx * (j + ny * k)
 * This provides efficient row-major access to the grid points.
 */
double& Solution::operator()(size_t i, size_t j, size_t k) {
    return data[i + nx * (j + ny * k)];
}

/**
 * @brief Implementation of const grid access operator
 * 
 * Provides read-only access to grid points using the same
 * indexing formula as the non-const operator.
 */
const double& Solution::operator()(size_t i, size_t j, size_t k) const {
    return data[i + nx * (j + ny * k)];
}

/**
 * @brief Implementation of grid initialization
 * 
 * Iterates through all grid points in a nested loop structure,
 * computing physical coordinates from grid indices and calling
 * the provided function g to set initial values.
 */
void Solution::initialize(std::function<double(double,double,double)> g) {
    for(size_t k = 0; k <= nz; ++k) {
        for(size_t j = 0; j <= ny; ++j) {
            for(size_t i = 0; i <= nx; ++i) {
                (*this)(i, j, k) = g(i * dx, j * dy, k * dz);
            }
        }
    }
}

void Solution::initialize(MTL::Buffer* buffer, size_t size) {
    // Vérifie que la taille est correcte
    if (size != data.size() * sizeof(float)) {
        throw std::runtime_error("Buffer size mismatch in GPU initialization");
    }
    
    // Copie les données depuis le buffer GPU
    float* gpu_data = static_cast<float*>(buffer->contents());
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<double>(gpu_data[i]);
    }
}

/**
 * @brief Implementation of solution swap
 * 
 * Uses the vector's swap method for efficient data exchange
 * without copying the entire arrays.
 */
void Solution::swap(Solution& other) {
    data.swap(other.data);
}