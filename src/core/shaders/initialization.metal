kernel void initialize_solution_kernel(device float* solution [[buffer(0)]],
                                device const Parameters& params [[buffer(1)]],
                                uint3 index [[thread_position_in_grid]]) {
    // Vérification des limites
    if (index.x > params.nx || index.y > params.ny || index.z > params.nz) {
        return;
    }
    
    // Calcul des coordonnées physiques
    float x = index.x * params.dx;
    float y = index.y * params.dy;
    float z = index.z * params.dz;
    
    // Calcul de l'indice 1D dans le buffer
    uint idx = index.x + params.nx * (index.y + params.ny * index.z);
    
    // Appel de la fonction d'initialisation parsée
    solution[idx] = g(x, y, z);
}