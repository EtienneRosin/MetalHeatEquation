#include "common.metal"

kernel void heat_equation_kernel(device const float* current_state [[ buffer(0) ]],
                               device float* next_state [[ buffer(1) ]],
                               device const Parameters& params [[ buffer(2) ]],
                               uint3 position [[thread_position_in_grid]])
{
    const uint i = position.x;
    const uint j = position.y;
    const uint k = position.z;
    
    // Skip boundary points
    if (i == 0 || i >= params.nx-1 || 
        j == 0 || j >= params.ny-1 || 
        k == 0 || k >= params.nz-1) {
        return;
    }
    
    const uint idx = i + params.nx * (j + params.ny * k);
    
    // Compute spatial indices for neighbors
    const uint idx_im1 = (i-1) + params.nx * (j + params.ny * k);
    const uint idx_ip1 = (i+1) + params.nx * (j + params.ny * k);
    const uint idx_jm1 = i + params.nx * ((j-1) + params.ny * k);
    const uint idx_jp1 = i + params.nx * ((j+1) + params.ny * k);
    const uint idx_km1 = i + params.nx * (j + params.ny * (k-1));
    const uint idx_kp1 = i + params.nx * (j + params.ny * (k+1));
    
    // Compute laplacian
    const float laplacian = 
        (current_state[idx_ip1] - 2*current_state[idx] + current_state[idx_im1]) / params.dx2 +
        (current_state[idx_jp1] - 2*current_state[idx] + current_state[idx_jm1]) / params.dy2 +
        (current_state[idx_kp1] - 2*current_state[idx] + current_state[idx_km1]) / params.dz2;
    
    // Compute force term
    const float x = i * params.dx;
    const float y = j * params.dy;
    const float z = k * params.dz;
    const float force = f(x, y, z, params.current_time);
    
    // Update state
    const float local_variation = params.dt * (laplacian + force);
    next_state[idx] = current_state[idx] + local_variation;
}