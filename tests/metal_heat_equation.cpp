#include "metal_heat_equation.hpp"
#include <Foundation/Foundation.hpp>

struct GPUParameters {
    float dx, dy, dz;
    float dx2, dy2, dz2;
    float dt;
    uint32_t nx, ny, nz;
    float current_time;
};

MetalHeatEquation::MetalHeatEquation(Parameters params,
                                   std::function<double(double,double,double,double)> f,
                                   std::function<double(double,double,double)> g)
    : HeatEquation(params, f, g)
{
    // Ajouter le timer pour le GPU
    timers.add("GPU Computation");
    
    initializeMetal();
    setupBuffers();
}

MetalHeatEquation::~MetalHeatEquation() {
    pipelineState->release();
    pipelineStateVariation->release();
    pipelineStateReduce->release();
    kernelFunction->release();
    library->release();
    currentBuffer->release();
    nextBuffer->release();
    paramsBuffer->release();
    variationBuffer->release();
    resultBuffer->release();
    commandQueue->release();
    device->release();
}

void MetalHeatEquation::initializeMetal() {
    // Get GPU device
    device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        throw std::runtime_error("No Metal-capable GPU device found");
    }

    // Create command queue
    commandQueue = device->newCommandQueue();
    if (!commandQueue) {
        throw std::runtime_error("Failed to create command queue");
    }

    // Read and convert force function
    ForceParser::ParsedForce parsedForce;
    try {
        parsedForce = ForceParser::parseForceFile("/Users/etienne/Documents/Developer/MetalHeat3D/src/config/force.hpp");
    } catch (const std::exception& e) {
        std::cerr << "Error processing force.hpp: " << e.what() << std::endl;
        throw;
    }

    // Build complete shader
    std::string shaderSource = R"(
#include <metal_stdlib>
using namespace metal;

struct Parameters {
    float dx, dy, dz;
    float dx2, dy2, dz2;
    float dt;
    uint32_t nx, ny, nz;
    float current_time;
};

)" + parsedForce.metalCode + R"(

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

kernel void compute_variation_kernel(device const float* current_state [[ buffer(0) ]],
                                  device const float* next_state [[ buffer(1) ]],
                                  device float* variation_buffer [[ buffer(3) ]],
                                  device const Parameters& params [[ buffer(2) ]],
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

kernel void reduce_variation_kernel(device const float* variation_buffer [[ buffer(0) ]],
                                device float* result [[ buffer(1) ]],
                                uint thread_position_in_grid [[ thread_position_in_grid ]],
                                uint threads_per_grid [[ threads_per_grid ]],
                                uint thread_position_in_threadgroup [[ thread_position_in_threadgroup ]],
                                uint threads_per_threadgroup [[ threads_per_threadgroup ]])
{
    threadgroup float shared_memory[256];
    
    uint tid = thread_position_in_threadgroup;
    uint i = thread_position_in_grid;
    
    if (i < threads_per_grid) {
        shared_memory[tid] = variation_buffer[i];
    } else {
        shared_memory[tid] = 0.0f;
    }
    
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    for (uint s = threads_per_threadgroup/2; s > 0; s >>= 1) {
        if (tid < s) {
            shared_memory[tid] += shared_memory[tid + s];
        }
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }
    
    if (tid == 0) {
        result[thread_position_in_grid / threads_per_threadgroup] = shared_memory[0];
    }
}
)";

    // Compile shader
    NS::Error* error = nullptr;
    auto source = NS::String::string(shaderSource.c_str(), NS::UTF8StringEncoding);
    auto options = MTL::CompileOptions::alloc()->init();
    
    library = device->newLibrary(source, options, &error);
    if (!library) {
        std::string errorMsg = error ? error->localizedDescription()->utf8String() : "Unknown error";
        throw std::runtime_error("Failed to compile Metal library: " + errorMsg);
    }

    // Get kernel functions
    auto functionName = NS::String::string("heat_equation_kernel", NS::UTF8StringEncoding);
    kernelFunction = library->newFunction(functionName);
    if (!kernelFunction) {
        throw std::runtime_error("Failed to load kernel function");
    }

    // Create pipeline states
    pipelineState = device->newComputePipelineState(kernelFunction, &error);
    if (!pipelineState) {
        throw std::runtime_error("Failed to create pipeline state");
    }

    auto variationFunction = library->newFunction(NS::String::string("compute_variation_kernel", NS::UTF8StringEncoding));
    pipelineStateVariation = device->newComputePipelineState(variationFunction, &error);
    if (!pipelineStateVariation) {
        throw std::runtime_error("Failed to create variation pipeline state");
    }

    auto reduceFunction = library->newFunction(NS::String::string("reduce_variation_kernel", NS::UTF8StringEncoding));
    pipelineStateReduce = device->newComputePipelineState(reduceFunction, &error);
    if (!pipelineStateReduce) {
        throw std::runtime_error("Failed to create reduce pipeline state");
    }
}

void MetalHeatEquation::setupBuffers() {
    const size_t dataSize = params.getNtot() * sizeof(float);
    
    // Create buffers for current and next state
    currentBuffer = device->newBuffer(dataSize, MTL::ResourceStorageModeShared);
    nextBuffer = device->newBuffer(dataSize, MTL::ResourceStorageModeShared);
    
    // Initialize data from CPU solution
    float* current_data = static_cast<float*>(currentBuffer->contents());
    for (size_t i = 0; i < params.getNtot(); ++i) {
        current_data[i] = static_cast<float>(U_current.get_data()[i]);
    }
    
    // Create buffer for parameters
    GPUParameters gpuParams{
        static_cast<float>(params.getDx()),
        static_cast<float>(params.getDy()),
        static_cast<float>(params.getDz()),
        static_cast<float>(params.getDx2()),
        static_cast<float>(params.getDy2()),
        static_cast<float>(params.getDz2()),
        static_cast<float>(params.getDt()),
        static_cast<uint32_t>(params.getNx()),
        static_cast<uint32_t>(params.getNy()),
        static_cast<uint32_t>(params.getNz()),
        0.0f
    };
    
    paramsBuffer = device->newBuffer(&gpuParams, sizeof(GPUParameters), MTL::ResourceStorageModeShared);

    // Création des buffers pour le calcul de variation
    const size_t num_interior_points = (params.getNx() - 2) * (params.getNy() - 2) * (params.getNz() - 2);
    variationBuffer = device->newBuffer(num_interior_points * sizeof(float), MTL::ResourceStorageModeShared);
    
    const size_t num_reduction_groups = (num_interior_points + 255) / 256;  // 256 threads par groupe
    resultBuffer = device->newBuffer(num_reduction_groups * sizeof(float), MTL::ResourceStorageModeShared);
    debugBuffer = device->newBuffer(3 * sizeof(float), MTL::ResourceStorageModeShared);
}

double MetalHeatEquation::compute_timestep() {
    timers("GPU Computation").start();
    
    auto gpuParams = static_cast<GPUParameters*>(paramsBuffer->contents());
    gpuParams->current_time = static_cast<float>(current_time);
    
    // Premier kernel : calcul de l'équation de la chaleur
    auto commandBuffer = commandQueue->commandBuffer();
    auto computeEncoder = commandBuffer->computeCommandEncoder();
    
    computeEncoder->setComputePipelineState(pipelineState);
    computeEncoder->setBuffer(currentBuffer, 0, 0);
    computeEncoder->setBuffer(nextBuffer, 0, 1);
    computeEncoder->setBuffer(paramsBuffer, 0, 2);
    
    MTL::Size gridSize = MTL::Size(params.getNx(), params.getNy(), params.getNz());
    MTL::Size threadgroupSize = MTL::Size(8, 8, 8);
    
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);
    computeEncoder->endEncoding();
    
    // Deuxième kernel : calcul des variations locales
    computeEncoder = commandBuffer->computeCommandEncoder();
    computeEncoder->setComputePipelineState(pipelineStateVariation);
    computeEncoder->setBuffer(currentBuffer, 0, 0);
    computeEncoder->setBuffer(nextBuffer, 0, 1);
    computeEncoder->setBuffer(paramsBuffer, 0, 2);
    computeEncoder->setBuffer(variationBuffer, 0, 3);
    computeEncoder->setBuffer(debugBuffer, 0, 4);
    
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);
    computeEncoder->endEncoding();
    
    // Troisième kernel : réduction pour calculer la somme totale
    computeEncoder = commandBuffer->computeCommandEncoder();
    computeEncoder->setComputePipelineState(pipelineStateReduce);
    
    uint32_t total_elements = (params.getNx() - 2) * (params.getNy() - 2) * (params.getNz() - 2);
    uint32_t threads_per_group = 256;
    uint32_t num_groups = (total_elements + threads_per_group - 1) / threads_per_group;
    
    MTL::Size reduceGridSize = MTL::Size(total_elements, 1, 1);
    MTL::Size reduceThreadgroupSize = MTL::Size(threads_per_group, 1, 1);
    
    computeEncoder->setBuffer(variationBuffer, 0, 0);
    computeEncoder->setBuffer(resultBuffer, 0, 1);
    
    computeEncoder->dispatchThreads(reduceGridSize, reduceThreadgroupSize);
    computeEncoder->endEncoding();
    
    // Exécuter et attendre
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
    
    // float* debug_values = static_cast<float*>(debugBuffer->contents());
    // std::cout << "GPU Debug at (1,1,1):" << std::endl;
    // std::cout << "  variation: " << debug_values[0] << std::endl;
    // std::cout << "  laplacian: " << debug_values[1] << std::endl;
    // std::cout << "  force: " << debug_values[2] << std::endl;
    
    // Lire le résultat final
    float* result = static_cast<float*>(resultBuffer->contents());
    double total_variation = 0.0;
    for (uint32_t i = 0; i < num_groups; ++i) {
        total_variation += result[i];
    }
    
    // Swap buffers
    std::swap(currentBuffer, nextBuffer);
    
    timers("GPU Computation").stop();
    
    return total_variation;
}