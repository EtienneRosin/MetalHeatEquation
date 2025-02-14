# MetalHeat3D
Project that solves the 3D Heat equation both sequentially (CPU) and using GPU acceleration via the Metal-cpp library, which interfaces with the GPU using the Metal Shading Language (MSL).

## Introduction

### The problem
We solve the Heat equation on $\Omega = [0, 1]^3$ 
$$
\begin{equation}
    \left|\begin{array}{l}
        \text{Find } u : (\boldsymbol{x}, t) \mapsto u(\boldsymbol{x}, t) \text{ such that :} \\[0.5em]
        \begin{aligned}
            \quad \partial_t u(\boldsymbol{x}, t) - \Delta u(\boldsymbol{x}, t) &= f(\boldsymbol{x}, t), &&\text{in } \Omega\times [0, T]\\
            \quad u(\boldsymbol{x}, 0) &= g(\boldsymbol{x}), &&\text{on } \partial\Omega\\
            \quad u(\boldsymbol{x}, t) &= g(\boldsymbol{x}), &&\text{on } \partial\Omega\times ]0, T]
        \end{aligned}
    \end{array}
    \right.
\end{equation}
$$
with $f$ and $g$ given.

### Discretization
We discretize (1) using the method of lines: 
- Spatial semi-discretization of order 2 (discretized in $n_x \times n_y \times n_z$ subdivisions)
- Euler explicit scheme in time

The integration recurrence relation is given by:
$$
\begin{equation}
    \displaystyle\boldsymbol{U}_{i,j,k}^{n+1} = \boldsymbol{U}_{i,j,k}^{n} + \Delta t\left(\left(\mathbb{D}\boldsymbol{U}^{n}\right)_{i,j,k} + f(\boldsymbol{x}_{i,j,k}, t_n) \right)
\end{equation}
$$
with:
* $\displaystyle \left(\mathbb{D}\boldsymbol{U}^{n}\right)_{i,j,k} = \frac{\boldsymbol{U}_{i+1,j,k}^{n} -2\boldsymbol{U}_{i,j,k}^{n} + \boldsymbol{U}_{i-1,j,k}^{n}}{\Delta x^2} + \frac{\boldsymbol{U}_{i,j+1,k}^{n} -2\boldsymbol{U}_{i,j,k}^{n} + \boldsymbol{U}_{i,j-1,k}^{n}}{\Delta y^2} + \frac{\boldsymbol{U}_{i,j,k+1}^{n} -2\boldsymbol{U}_{i,j,k}^{n} + \boldsymbol{U}_{i,j,k+1}^{n}}{\Delta x^2}$
* $\displaystyle\boldsymbol{x}_{i,j,k} = \left(i\Delta x, j\Delta y, k\Delta z\right)^T$
* $\displaystyle (i, j, k) \in \llbracket 0, n_x \rrbracket\times\llbracket 0, n_y \rrbracket\times\llbracket 0, n_z \rrbracket$

### Stability condition
The Von-Neumann analysis leads to the following CFL condition:
$$
\begin{equation}
    \displaystyle \Delta t \leq \frac{1}{2}\left(\Delta x^2 + \Delta y^2 + \Delta z^2\right)
\end{equation}
$$

## Implementation

### CPU Version
The CPU implementation (`HeatEquation` class) provides a reference sequential solution:
- Direct implementation of the numerical scheme
- Optimized for clarity and correctness
- Uses C++ STL containers for data storage

### GPU Version (Metal)
The GPU implementation (`MetalHeatEquation` class) accelerates computation using Apple's Metal framework:
- Utilizes 3D grid computation with configurable thread group size (default: 8x8x8)
- Automatic conversion of C++ force functions to MSL
- Three-stage computation pipeline:
  1. Heat equation kernel: Computes next state
  2. Variation kernel: Calculates local variations
  3. Reduction kernel: Aggregates variations using parallel reduction

## Configuration

### Parameters
- `f(x,y,z,t)`: Force function
- `g(x,y,z)`: Initial/boundary condition function
- `nx, ny, nz`: Number of subdivisions in each direction
- `dx, dy, dz`: Spatial steps (derived from subdivisions)
- `T`: Final time
- `nt`: Number of time steps
- `dt`: Time step (derived from T and nt, must satisfy CFL condition)
- `freq`: Output frequency for monitoring convergence

### Force Function
The force function must be defined in `src/config/force.hpp` following this template:
```cpp
inline double f(double x, double y, double z, double t) {
    // Your force function implementation
    return ...;
}
```

## Performance
The GPU implementation typically achieves significant speedup compared to the CPU version:
- Efficient parallel computation of the 3D grid
- Optimized memory access patterns
- Parallel reduction for variation computation

## Building and Running
Prerequisites:
- macOS Sonoma or later
- Xcode with Metal support
- CMake 3.20 or later

Building:
```bash
mkdir build
cd build
cmake ..
make
```

## References
1. Strikwerda, J. C. (2004). Finite difference schemes and partial differential equations (Vol. 88). Siam.
2. [Metal Programming Guide, Apple Inc.](https://developer.apple.com/documentation/metal?language=objc)