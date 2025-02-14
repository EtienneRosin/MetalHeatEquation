#include "common.metal"

kernel void compute_variation_kernel(device const float* current_state [[ buffer(0) ]],
                                  device const float* next_state [[ buffer(1) ]],
                                  device const Parameters& params [[ buffer(2) ]],
                                  device float* variation_buffer [[ buffer(3) ]],
                                  device float* debug_buffer [[ buffer(4) ]],
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
    
    // Calculer l'index dans le buffer de variation pour les points intérieurs uniquement
    const uint interior_i = i - 1;
    const uint interior_j = j - 1;
    const uint interior_k = k - 1;
    const uint interior_nx = params.nx - 2;
    const uint interior_ny = params.ny - 2;
    const uint grid_idx = interior_i + interior_nx * (interior_j + interior_ny * interior_k);
    
    // Calcul similaire à la version CPU
    const float x = i * params.dx;
    const float y = j * params.dy;
    const float z = k * params.dz;
    
    const float um = current_state[idx];
    const float up1 = current_state[idx + 1];
    const float um1 = current_state[idx - 1];
    const float vp1 = current_state[idx + params.nx];
    const float vm1 = current_state[idx - params.nx];
    const float wp1 = current_state[idx + params.nx * params.ny];
    const float wm1 = current_state[idx - params.nx * params.ny];
    
    const float laplacian = 
        (up1 - 2*um + um1) / params.dx2 +
        (vp1 - 2*um + vm1) / params.dy2 +
        (wp1 - 2*um + wm1) / params.dz2;
    
    const float force = f(x, y, z, params.current_time);
    const float local_variation = params.dt * (laplacian + force);
    variation_buffer[grid_idx] = abs(local_variation);
    
    // Debug pour le point (1,1,1)
    if (i == 1 && j == 1 && k == 1) {
        debug_buffer[0] = local_variation;
        debug_buffer[1] = laplacian;
        debug_buffer[2] = force;
    }
}