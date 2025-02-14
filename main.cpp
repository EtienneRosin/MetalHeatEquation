/**
 * @file main.cpp
 * @brief Main program for solving heat equations on CPU and GPU
 * @author Etienne Rosin
 * @date February 13, 2025
 * 
 * This program implements the numerical solution of heat equations using
 * both a traditional CPU approach and a GPU approach via Metal.
 * It allows performance comparison between the two methods.
 */

#include "parameters.hpp"
#include "solution.hpp"
#include "force.hpp"
#include "initial_condition.hpp"
#include "heat_equation.hpp"
#include "metal_heat_equation.hpp"
#include "metal_device_info.hpp"

/**
 * @brief Main entry point of the program
 * @return Program return code (0 if successful)
 * 
 * The main program performs the following operations:
 * 1. Displays information about available Metal devices
 * 2. Loads simulation parameters from a file
 * 3. Executes the heat equation solution on CPU
 * 4. Executes the heat equation solution on GPU (Metal)
 * 
 * Functions f and g represent the source term
 * and initial condition of the heat equation respectively.
 */
int main() {
    // Display Metal device information
    MetalDeviceInfo deviceInfo;
    deviceInfo.displayAllDevicesInfo();

    // Load parameters from configuration file
    //  Parameters params("src/config/parameters.txt");
    #ifndef CONFIG_PATH
        #define CONFIG_PATH "."  // Valeur par défaut pour l'éditeur
    #endif
    Parameters params(std::string(CONFIG_PATH) + "/parameters.txt");
    params.print();

    // CPU solution
    // HeatEquation cpu_equation(params, f, g);
    // std::cout << "Begin solving CPU ───────────────────────────────────────────"<< std::endl;
    // cpu_equation.solve();
    // cpu_equation.timers.display();

    // // GPU solution using Metal
    MetalHeatEquation metal_equation(params, f, g);
    std::cout << "Begin solving GPU ───────────────────────────────────────────" << std::endl;
    metal_equation.solve();
    metal_equation.timers.display();

    return 0;
}

/**
 * Note: The commented lines in the original source code
 * show testing functionality that may be useful
 * for debugging purposes:
 * 
 * // std::cout << "f : " << f(0.1, 0.1, 0.1, 0) << std::endl;
 * // std::cout << "g : " << g(0.5, 0.5, 0.5) << std::endl;
 * // Solution U(params);
 * // U.initialize(g);
 */