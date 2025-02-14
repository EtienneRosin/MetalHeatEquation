/**
 * @file solution.hpp
 * @brief Solution class for managing 3D grid data in heat equation simulations
 * @author Etienne Rosin
 * @date February 13, 2025
 * 
 * This class handles the storage and manipulation of 3D grid data for heat equation
 * simulations. It provides functionality for initialization, access, and data management
 * with proper spatial discretization.
 */

#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include <vector>
#include <functional>
#include <fstream>
#include <string>
#include <cassert>
#include <cstddef>
#include "parameters.hpp"
#include <Metal/Metal.hpp>

/**
 * @class Solution
 * @brief Manages a 3D grid solution for heat equation simulations
 * 
 * The Solution class provides a container and operations for 3D grid data,
 * including initialization from functions and efficient data access methods.
 * It maintains the spatial discretization parameters and grid dimensions.
 */
class Solution {
private:
    std::vector<double> data;              ///< Storage for grid point values
    Parameters params;                      ///< Simulation parameters
    const double dx, dy, dz;               ///< Grid spacing in each dimension
    const double dx2, dy2, dz2;            ///< Squared grid spacing for computations
    const size_t nx, ny, nz;               ///< Number of grid points in each dimension

public:
    /**
     * @brief Constructor
     * @param params Reference to simulation parameters
     * 
     * Initializes a solution grid with dimensions and spacing defined
     * in the provided parameters. Allocates memory for the entire grid.
     */
    Solution(Parameters &params);
    
    /**
     * @brief Grid point access operator
     * @param i x-direction index
     * @param j y-direction index
     * @param k z-direction index
     * @return Reference to the value at the specified grid point
     * 
     * Provides direct access to grid points using 3D indexing.
     * Converts 3D indices to the appropriate 1D array position.
     */
    double& operator()(size_t i, size_t j, size_t k);

    /**
     * @brief Const grid point access operator
     * @param i x-direction index
     * @param j y-direction index
     * @param k z-direction index
     * @return Const reference to the value at the specified grid point
     * 
     * Provides read-only access to grid points using 3D indexing.
     */
    const double& operator()(size_t i, size_t j, size_t k) const;
    
    /**
     * @brief Initializes the grid using a function
     * @param g Function that provides initial values based on spatial coordinates
     * 
     * Fills the entire grid with values computed from the provided function g,
     * which takes (x,y,z) coordinates and returns the initial value at that point.
     */
    void initialize(std::function<double(double,double,double)> g);

    /**
     * @brief Initializes the grid using metal
     */
    void initialize(MTL::Buffer* buffer, size_t size);

    /**
     * @brief Swaps contents with another Solution object
     * @param other Solution object to swap with
     * 
     * Efficiently exchanges the data between two Solution objects.
     * Useful for updating solutions without copying large arrays.
     */
    void swap(Solution& other);
    
    /**
     * @brief Gets raw pointer to data array
     * @return Pointer to the underlying data array
     */
    double* get_data() { return data.data(); }

    /**
     * @brief Gets const raw pointer to data array
     * @return Const pointer to the underlying data array
     */
    const double* get_data() const { return data.data(); }
};

#endif