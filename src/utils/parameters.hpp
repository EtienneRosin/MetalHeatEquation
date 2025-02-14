/**
 * @file parameters.hpp
 * @brief Configuration parameters management for numerical simulation
 * @author [Your Name]
 * @date February 12, 2025
 * 
 * This utility provides a class for managing simulation parameters
 * loaded from a configuration file. It handles parameter parsing,
 * validation, and access.
 */
#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <cmath>



/**
 * @class Parameters
 * @brief Manages simulation configuration parameters
 * 
 * The Parameters class provides functionality to load and manage
 * simulation parameters from a configuration file. It handles parameter
 * parsing, type conversion, and provides accessor methods for all parameters.
 */
class Parameters {
private:
    // Spatial parameters
    int n_x, n_y, n_z;                          ///< Number of points in each direction
    int n_tot;                                  ///< Total number of spatial points
    double dx, dy, dz;                          ///< Spatial steps in each direction
    double dx2, dy2, dz2;                       ///< Squared spatial steps

    // Temporal parameters
    int max_iterations;                         ///< Number of iterations to perform
    double dt;                                  ///< Time step
    double T;                                   ///< Total simulation time (computed as max_iterations * dt)
    
    int output_frequency;                       ///< Frequency of output saves
    std::map<std::string, std::string> params;  ///< Storage for raw parameter values

    /**
     * @brief Calculates spatial steps from grid parameters
     * 
     * Computes dx, dy, dz assuming a [0,1] domain in each direction
     */
    void computeSpatialSteps() {
        dx = 1.0 / n_x;
        dy = 1.0 / n_y;
        dz = 1.0 / n_z;
        dx2 = dx * dx;
        dy2 = dy * dy;
        dz2 = dz * dz;
    }

    /**
     * @brief Checks CFL condition
     * 
     * Verifies the CFL stability condition
     */
    void checkCFLCondition() {
        // Check the CFL condition
        // double cfl_limit = 0.5 * std::min(dx2, std::min(dy2, dz2));
        double cfl_limit = 0.1 * std::min(dx2, std::min(dy2, dz2));
        // 0.25 to have a factor of safety of 5
        // double cfl_limit = 0.5 * (dx2 + dy2 + dz2);

        if (dt > cfl_limit) {
            std::cerr << "\nWARNING: CFL condition not satisfied!\n"
                    << "Current dt = " << dt << "\n"
                    << "Maximum stable dt = " << cfl_limit << "\n"
                    << "Simulation might be unstable!\n" << std::endl;
        }
    }

public:
    /**
     * @brief Constructor that loads parameters from file
     * @param filename Path to the configuration file
     * @throw std::runtime_error if file cannot be opened or parameters are invalid
     */
    Parameters(const std::string& filename) {
        readFromFile(filename);
        computeSpatialSteps();
        checkCFLCondition();
    }

    /**
     * @brief Reads and parses the configuration file
     * @param filename Path to the configuration file
     * @throw std::runtime_error if file cannot be opened or parameter parsing fails
     */
    void readFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Impossible to open the file " + filename);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                params[key] = value;
            }
        }

        try {
            n_x = std::stoi(params["nx"]);
            n_y = std::stoi(params["ny"]);
            n_z = std::stoi(params["nz"]);
            n_tot = (n_x + 1) * (n_y + 1) * (n_z + 1);
            dt = std::stod(params["dt"]);
            max_iterations = std::stoi(params["max_iterations"]);
            T = dt * max_iterations;
            output_frequency = std::stoi(params["output_frequency"]);
        } catch (const std::exception& e) {
            throw std::runtime_error("Error while parsing parameters: " + std::string(e.what()));
        }
    }

    // Getters existants
    const size_t getNx() const { return n_x; }
    const size_t getNy() const { return n_y; }
    const size_t getNz() const { return n_z; }
    const size_t getNtot() const { return n_tot; }
    double getT() const { return T; }
    int getMaxIterations() const { return max_iterations; }
    int getOutputFrequency() const { return output_frequency; }

    // Nouveaux getters
    double getDx() const { return dx; }
    double getDx2() const { return dx2; }
    double getDy() const { return dy; }
    double getDy2() const { return dy2; }
    double getDz() const { return dz; }
    double getDz2() const { return dz2; }
    double getDt() const { return dt; }

    /**
     * @brief Displays all current parameter values
     * 
     * Outputs all simulation parameters to standard output in a formatted way
     */
    // void print() const {
    //     std::cout << "Simulation parameters:\n";
    //     std::cout << "\nGrid parameters:\n";
    //     std::cout << " | nx = " << n_x << " (dx = " << dx << ")\n";
    //     std::cout << " | ny = " << n_y << " (dy = " << dy << ")\n";
    //     std::cout << " | nz = " << n_z << " (dz = " << dz << ")\n";
    //     std::cout << "\nTime parameters:\n";
    //     std::cout << " | max_iterations = " << max_iterations << "\n";
    //     std::cout << " | dt = " << dt << "\n";
    //     std::cout << " | T = " << T << "\n";
    //     std::cout << "\nOutput parameters:\n";
    //     std::cout << " | Output frequency = " << output_frequency << "\n";
    // }
    void print() const {
        // // std::cout << std::left
        // //               << std::setw(8) << iter 
        // //               << std::scientific << std::setprecision(3)
        // //               << std::setw(15) << current_time 
        // //               << std::setw(20) << variation 
        // //               << std::fixed << std::setw(15) << timers("Calculation").get_elapsed()
        // //               << std::endl;


        // std::cout << "┌───────────────────────────────────────┐\n";
        // std::cout << "│         Simulation Parameters         │\n";
        // std::cout << "├────────────────┬──────────────────────┤\n";
        // std::cout << "│       Grid     │      Time            │\n";
        // std::cout << "├────────────────┼──────────────────────┤\n";
        // std::cout << "│ nx = " << std::setw(4) << n_x << "     │ Nt = " 
        //           << std::scientific << std::setprecision(1) << std::setw(5) << max_iterations << "      │\n";
        // std::cout << "│ ny = " << std::setw(4) << n_y << "     │ dt = " 
        //           << std::scientific << std::setprecision(1) << std::setw(9) << dt << "    │\n";
        // std::cout << "│ nz = " << std::setw(4) << n_z << "     │ T = " 
        //           << std::scientific << std::setprecision(1) << std::setw(9) << T << "    │\n";
        // std::cout << "├────────────────┴──────────────────────┤\n";
        // std::cout << "│ Output Frequency: " << std::setw(10) << output_frequency << "         │\n";
        // std::cout << "└───────────────────────────────────────┘\n";



        // std::cout << std::scientific << std::setprecision(1)
        //           << std::setw(4) << "│ nx = " << std::setw(7) << n_x << " │ " << std::setw(6) << "Nt = " << std::setw(7) << max_iterations << " │\n"
        //           << std::setw(4) << "│ ny = " << std::setw(7) << n_y << " │ " << std::setw(6) << "dt = " << std::setw(7) << dt << " │\n"
        //           << std::setw(4) << "│ nz = " << std::setw(7) << n_z  << " │ " << std::setw(6) << "T = " << std::setw(7) << T << " │\n";
        // std::cout << "│ " << std::setw(17) << "Output Frequency:" << std::setw(11) << output_frequency << " │\n";
        size_t width = 31;
        const size_t inner_width = width - 2;
        auto center_text = [](const std::string& text, size_t column_width) {
            size_t padding = column_width - text.length();
            size_t left_pad = padding / 2;
            size_t right_pad = padding - left_pad;
            return std::string(left_pad, ' ') + text + std::string(right_pad, ' ');
        };
        // std::cout << "└" << std::string(width, '-') << "┘\n";

        std::string hline(inner_width, '-');
    
        // En-tête
        std::cout << "+" << hline << "+\n"
                << "|" << center_text("Simulation Parameters", inner_width) << "|\n"
                << "+" << hline << "+\n"
                << "|" << center_text("Grid", 13) << std::setw(3) << " | " << center_text("Time", 13) << "|\n"
                << "+" << std::string(14, '-') << "+" << std::string(14, '-') << "+\n";
        
        // Données de la grille et du temps
        std::cout << std::scientific << std::setprecision(1)
                << "| nx = " << std::setw(7) << n_x << " | Nt = "<< std::scientific << std::setprecision(1) << std::setw(7) << max_iterations << " |\n"
                << "| ny = " << std::setw(7) << n_y << " | dt = " << std::setw(7) << dt << " |\n"
                << "| nz = " << std::setw(7) << n_z << " | T  = " << std::setw(7) << T << " |\n";
        
        // Ligne de séparation et fréquence de sortie
        std::cout << "+" << std::string(width - 2, '-') << "+\n"
                // << "| Output Frequency: " << std::setw(9) << output_frequency << " |\n"
                << "|" << center_text("Output Frequency: " + std::to_string(output_frequency), inner_width) << "|\n"
                << "+" << std::string(width - 2, '-') << "+\n";
    }
};
// 1
// | nz =     201 | T  = 3.0e-06 |
// nx =  201     ,
// Nt =    10      ,
// dt =   3.0e-07, // 7
// │ nx =     201     │ Nt =      10      │
// │       Grid     │      Time            │

// 24 + 3
// Output Frequency:
