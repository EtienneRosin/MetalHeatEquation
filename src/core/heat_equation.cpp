#include "heat_equation.hpp"
#include <iostream>
#include <cmath>

HeatEquation::HeatEquation(Parameters params, 
                          std::function<double(double,double,double,double)> f,
                          std::function<double(double,double,double)> g, 
                          bool gpu_init)
    : params(params)
    , f(f)
    , current_time(0.0)
    , U_current(params)
    , U_next(params)
    , timers()
{
    timers.add("Calculation");
    timers.add("Others");
    timers.add("Initialization");

    if(!gpu_init){
        timers("Initialization").start();
        U_current.initialize(g);
        U_next.initialize(g);
        timers("Initialization").stop();
    }
    
}

double HeatEquation::compute_timestep() {
    const double dx = params.getDx();
    const double dx2 = params.getDx2();
    const double dy = params.getDy();
    const double dy2 = params.getDy2();
    const double dz = params.getDz();
    const double dz2 = params.getDz2();
    const double dt = params.getDt();
    const size_t nx = params.getNx();
    const size_t ny = params.getNy();
    const size_t nz = params.getNz();
    
    double total_variation = 0.0;

    // Compute inside the domain (not at the border)
    for (size_t k = 1; k < nz; ++k) {
        for (size_t j = 1; j < ny; ++j) {
            for (size_t i = 1; i < nx; ++i) {
                // Compute the discrete laplacian
                const double laplacian =
                    (U_current(i+1,j,k) - 2*U_current(i,j,k) + U_current(i-1,j,k)) / dx2 +
                    (U_current(i,j+1,k) - 2*U_current(i,j,k) + U_current(i,j-1,k)) / dy2 +
                    (U_current(i,j,k+1) - 2*U_current(i,j,k) + U_current(i,j,k-1)) / dz2;
                
                // Compute the force term with current time
                const double force = f(i * dx, j * dy, k * dz, current_time);
                
                const double local_variation = dt * (laplacian + force);
                // if (i == 1 && j == 1 && k == 1) {
                //     std::cout << "CPU Debug at (1,1,1):" << std::endl;
                //     std::cout << "  variation: " << local_variation << std::endl;
                //     std::cout << "  laplacian: " << laplacian << std::endl;
                //     std::cout << "  force: " << force << std::endl;
                // }

                U_next(i, j, k) = U_current(i, j, k) + local_variation;
                total_variation += std::abs(local_variation);
            }
        }
    }
    return total_variation;
}

void HeatEquation::solve() {
    const size_t max_iterations = params.getMaxIterations();
    const size_t output_frequency = params.getOutputFrequency();
    const double dt = params.getDt();
    double variation;
    // std::cout << "iteration,    simulation_time,    variation,    elapsed computation time(ms)" << std::endl;
    std::cout << std::left 
          << std::setw(8) << "Iter" 
          << std::setw(15) << "Sim Time" 
          << std::setw(15) << "Variation" 
          << std::setw(10) << "Comp Time (ms)" 
          << std::endl;
    
    for (size_t iter = 0; iter < max_iterations; ++iter) {
        timers("Calculation").start();
        variation = compute_timestep();
        timers("Calculation").stop();

        timers("Others").start();
        current_time += dt;
        U_current.swap(U_next);

        // if (output_frequency > 0 && iter % output_frequency == 0) {
        //     // std::cout << iter << ",    " << current_time << ",    " << variation << ",    " << timers("Calculation").get_elapsed() << std::endl;
        //     std::cout << std::left
        //     << std::setw(8) << iter 
        //     << std::setw(15) << current_time 
        //     << std::setw(15) << variation 
        //     << std::setw(10) << timers("Calculation").get_elapsed()
        //     << std::endl;
        //     // timers("Calculation").stop();
        //     // std::cout << "iteration: " << iter
        //     //          << ", time: " << current_time 
        //     //          << ", variation: " << variation
        //     //          << ", elapsed time: " << timers("Calculation").get_elapsed() << " ms" << std::endl;
        //     // timers("Calculation").start();
        // }
        if (output_frequency > 0 && iter % output_frequency == 0) {
            std::cout << std::left
                      << std::setw(8) << iter 
                      << std::scientific << std::setprecision(3)
                      << std::setw(15) << current_time 
                      << std::setw(20) << variation 
                      << std::fixed << std::setw(15) << timers("Calculation").get_elapsed()
                      << std::endl;
        }
        timers("Others").stop();
    }
    // timers("Calculation").stop();
    
}