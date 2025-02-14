#ifndef HEAT_EQUATION_HPP
#define HEAT_EQUATION_HPP

#include "parameters.hpp"
#include "solution.hpp"
#include "timer.hpp"
#include <functional>

class HeatEquation {
public:
    Timers timers;
protected:
    Parameters params;
    Solution U_current;
    Solution U_next;
    std::function<double(double, double, double, double)> f;
    double current_time;
    

    // Calcule une itération et retourne la variation maximale
    // double compute_timestep();
// protected:  // instead of private so that sub-classes can override it
    virtual double compute_timestep();

public:
    HeatEquation(Parameters params, 
                 std::function<double(double,double,double,double)> f,
                 std::function<double(double,double,double)> g,
                 bool gpu_init = false);

    const Solution& get_solution() const { return U_current; }
    double get_current_time() const { return current_time; }
    void solve();
};

#endif