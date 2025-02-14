#include "metal_heat_equation.hpp"
#include "shader_loader.hpp"
#include <Foundation/Foundation.hpp>

struct GPUParameters {
    float dx, dy, dz;
    float dx2, dy2, dz2;
    float dt;
    uint32_t nx, ny, nz;
    float current_time;
};

// MetalHeatEquation::MetalHeatEquation(Parameters params,
//                                    std::function<double(double,double,double,double)> f,
//                                    std::function<double(double,double,double)> g)
//     : HeatEquation(params, f, g)
// {
//     // Ajouter le timer pour le GPU
//     // timers.add("GPU Computation");
//     timers("Initialization").start();
//     initializeMetal();
//     setupBuffers();
//     timers("Initialization").stop();
// }
MetalHeatEquation::MetalHeatEquation(Parameters params,
    std::function<double(double,double,double,double)> f,
    std::function<double(double,double,double)> g)
: HeatEquation(params, f, g, true)  // true pour indiquer l'initialisation GPU
{
    // std::cout << "Starting MetalHeatEquation constructor" << std::endl;
    try {
        // std::cout << "Starting initialization" << std::endl;
        timers("Initialization").start();
        
        // std::cout << "Calling initializeMetal()" << std::endl;
        initializeMetal();
        
        // std::cout << "Calling setupBuffers()" << std::endl;
        setupBuffers();
        
        // std::cout << "Calling initializeSolutionGPU()" << std::endl;
        initializeSolutionGPU();
        
        timers("Initialization").stop();
        // std::cout << "Initialization completed successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught in constructor: " << e.what() << std::endl;
        throw;
    }
}


void MetalHeatEquation::initializeSolutionGPU() {
    auto commandBuffer = commandQueue->commandBuffer();
    auto computeEncoder = commandBuffer->computeCommandEncoder();
    
    computeEncoder->setComputePipelineState(pipelineStateInit);
    computeEncoder->setBuffer(currentBuffer, 0, 0);
    computeEncoder->setBuffer(paramsBuffer, 0, 1);
    
    MTL::Size gridSize = MTL::Size(params.getNx() + 1, params.getNy() + 1, params.getNz() + 1);
    MTL::Size threadgroupSize = MTL::Size(8, 8, 8);
    
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);
    computeEncoder->endEncoding();
    
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    // Utiliser la nouvelle méthode initialize pour mettre à jour les solutions CPU
    const size_t dataSize = params.getNtot() * sizeof(float);
    U_current.initialize(currentBuffer, dataSize);
    U_next.initialize(currentBuffer, dataSize);  // On utilise le même buffer car même valeurs initiales
}

// MetalHeatEquation::~MetalHeatEquation() {
//     pipelineState->release();
//     pipelineStateVariation->release();
//     pipelineStateReduce->release();
//     kernelFunction->release();
//     library->release();
//     currentBuffer->release();
//     nextBuffer->release();
//     paramsBuffer->release();
//     variationBuffer->release();
//     resultBuffer->release();
//     commandQueue->release();
//     device->release();
// }

MetalHeatEquation::~MetalHeatEquation() {
    if (pipelineStateInit) pipelineStateInit->release();  // Ajouter cette ligne
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

// void MetalHeatEquation::initializeMetal() {
//     // Get GPU device
//     // Get GPU device
//     device = MTL::CreateSystemDefaultDevice();
//     if (!device) {
//         throw std::runtime_error("No Metal-capable GPU device found");
//     }

//     // Create command queue
//     commandQueue = device->newCommandQueue();
//     if (!commandQueue) {
//         throw std::runtime_error("Failed to create command queue");
//     }

//     // Configure et parse la fonction de force
//     FunctionParser::ParserOptions forceOptions;
//     forceOptions.functionName = "f";
//     forceOptions.requiredParams = {"double", "double", "double", "double"}; // x, y, z, t
//     forceOptions.requireInline = true;

//     // Configure et parse la condition initiale
//     FunctionParser::ParserOptions initOptions;
//     initOptions.functionName = "g";
//     initOptions.requiredParams = {"double", "double", "double"}; // x, y, z
//     initOptions.requireInline = true;

//     // Parse les deux fonctions
//     FunctionParser::ParsedFunction parsedForce;
//     FunctionParser::ParsedFunction parsedInit;
//     try {
//         parsedForce = FunctionParser::parseFile("../src/config/force.hpp", forceOptions);
//         parsedInit = FunctionParser::parseFile("../src/config/initial_condition.hpp", initOptions);
//     } catch (const std::exception& e) {
//         std::cerr << "Error processing functions: " << e.what() << std::endl;
//         throw;
//     }

//     // Load and combine all shaders
//     std::string shaderSource;
//     try {
//         shaderSource = ShaderLoader::loadShaders(parsedForce.metalCode, parsedInit.metalCode);
//     } catch (const std::exception& e) {
//         std::cerr << "Error loading shaders: " << e.what() << std::endl;
//         throw;
//     }

//     // Compile shader
//     NS::Error* error = nullptr;
//     auto source = NS::String::string(shaderSource.c_str(), NS::UTF8StringEncoding);
//     auto options = MTL::CompileOptions::alloc()->init();

//     // std::cout << "Force function: " << parsedForce.metalCode << std::endl;
//     // std::cout << "Final shader:\n" << shaderSource << std::endl;


//     // // Compile shader
//     // NS::Error* error = nullptr;
//     // auto source = NS::String::string(shaderSource.c_str(), NS::UTF8StringEncoding);
//     // auto options = MTL::CompileOptions::alloc()->init();
    
//     library = device->newLibrary(source, options, &error);
//     if (!library) {
//         std::string errorMsg = error ? error->localizedDescription()->utf8String() : "Unknown error";
//         throw std::runtime_error("Failed to compile Metal library: " + errorMsg);
//     }

//     // Get kernel functions
//     kernelFunction = library->newFunction(NS::String::string("heat_equation_kernel", NS::UTF8StringEncoding));
//     if (!kernelFunction) {
//         throw std::runtime_error("Failed to load heat equation kernel function");
//     }

//     auto variationFunction = library->newFunction(NS::String::string("compute_variation_kernel", NS::UTF8StringEncoding));
//     if (!variationFunction) {
//         throw std::runtime_error("Failed to load variation kernel function");
//     }

//     auto reduceFunction = library->newFunction(NS::String::string("reduce_variation_kernel", NS::UTF8StringEncoding));
//     if (!reduceFunction) {
//         throw std::runtime_error("Failed to load reduce kernel function");
//     }

//     // Create pipeline states
//     // pipelineState = device->newComputePipelineState(kernelFunction, &error);
//     // if (!pipelineState) {
//     //     throw std::runtime_error("Failed to create heat equation pipeline state");
//     // }
//     pipelineState = device->newComputePipelineState(kernelFunction, &error);
//     if (!pipelineState) {
//         std::string errorMsg = error ? error->localizedDescription()->utf8String() : "Unknown error";
//         std::cerr << "Pipeline state creation failed: " << errorMsg << std::endl;
//         throw std::runtime_error("Failed to create heat equation pipeline state: " + errorMsg);
//     }

//     pipelineStateVariation = device->newComputePipelineState(variationFunction, &error);
//     if (!pipelineStateVariation) {
//         throw std::runtime_error("Failed to create variation pipeline state");
//     }

//     pipelineStateReduce = device->newComputePipelineState(reduceFunction, &error);
//     if (!pipelineStateReduce) {
//         throw std::runtime_error("Failed to create reduce pipeline state");
//     }
// }
void MetalHeatEquation::initializeMetal() {
    // std::cout << "=== Starting initializeMetal() ===" << std::endl;
    
    // Get GPU device
    // std::cout << "Creating Metal device..." << std::endl;
    device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        throw std::runtime_error("No Metal-capable GPU device found");
    }
    // std::cout << "Metal device created successfully" << std::endl;

    // Create command queue
    // std::cout << "Creating command queue..." << std::endl;
    commandQueue = device->newCommandQueue();
    if (!commandQueue) {
        throw std::runtime_error("Failed to create command queue");
    }
    // std::cout << "Command queue created successfully" << std::endl;

    // Configure function parser for force function
    // std::cout << "Configuring force function parser options..." << std::endl;
    FunctionParser::ParserOptions forceOptions;
    forceOptions.functionName = "f";
    forceOptions.requiredParams = {"double", "double", "double", "double"}; // x, y, z, t
    forceOptions.requireInline = true;
    std::cout << "Force function parser options configured" << std::endl;

    // Configure function parser for initial condition
    // std::cout << "Configuring initial condition parser options..." << std::endl;
    FunctionParser::ParserOptions initOptions;
    initOptions.functionName = "g";
    initOptions.requiredParams = {"double", "double", "double"}; // x, y, z
    initOptions.requireInline = true;
    // std::cout << "Initial condition parser options configured" << std::endl;

    // Parse functions
    // std::cout << "Starting to parse functions..." << std::endl;
    FunctionParser::ParsedFunction parsedForce;
    FunctionParser::ParsedFunction parsedInit;
    try {
        // std::cout << "Attempting to parse force.hpp..." << std::endl;
        parsedForce = FunctionParser::parseFile("../src/config/force.hpp", forceOptions);
        // std::cout << "Force function parsed successfully" << std::endl;
        
        // std::cout << "Attempting to parse initial_condition.hpp..." << std::endl;
        parsedInit = FunctionParser::parseFile("../src/config/initial_condition.hpp", initOptions);
        // std::cout << "Initial condition parsed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error processing functions: " << e.what() << std::endl;
        throw;
    }

    // Load shaders
    // std::cout << "Loading and combining shaders..." << std::endl;
    // std::string shaderSource;
    // try {
    //     shaderSource = ShaderLoader::loadShaders(parsedForce.metalCode, parsedInit.metalCode);
    //     // std::cout << "Shaders loaded and combined successfully" << std::endl;
    // } catch (const std::exception& e) {
    //     std::cerr << "Error loading shaders: " << e.what() << std::endl;
    //     throw;
    // }
    std::string shaderSource;
    try {
        shaderSource = ShaderLoader::loadShaders(parsedForce.metalCode, parsedInit.metalCode);
        // Ajouter ce log
        // std::cout << "=== Final Shader Source ===" << std::endl;
        // std::cout << shaderSource << std::endl;
        // std::cout << "==========================" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading shaders: " << e.what() << std::endl;
        throw;
    }

    // Compile shader
    // std::cout << "Starting shader compilation..." << std::endl;
    NS::Error* error = nullptr;
    auto source = NS::String::string(shaderSource.c_str(), NS::UTF8StringEncoding);
    auto options = MTL::CompileOptions::alloc()->init();
    // std::cout << "Compilation options initialized" << std::endl;

    // std::cout << "Attempting to create Metal library..." << std::endl;
    library = device->newLibrary(source, options, &error);
    if (!library) {
        std::string errorMsg = error ? error->localizedDescription()->utf8String() : "Unknown error";
        std::cerr << "Failed to compile Metal library: " << errorMsg << std::endl;
        throw std::runtime_error("Failed to compile Metal library: " + errorMsg);
    }
    // std::cout << "Metal library created successfully" << std::endl;

    // Get kernel functions
    // std::cout << "Getting kernel functions..." << std::endl;
    
    // std::cout << "Loading heat equation kernel..." << std::endl;
    kernelFunction = library->newFunction(NS::String::string("heat_equation_kernel", NS::UTF8StringEncoding));
    if (!kernelFunction) {
        throw std::runtime_error("Failed to load heat equation kernel function");
    }
    // std::cout << "Heat equation kernel loaded" << std::endl;

    // std::cout << "Loading variation kernel..." << std::endl;
    auto variationFunction = library->newFunction(NS::String::string("compute_variation_kernel", NS::UTF8StringEncoding));
    if (!variationFunction) {
        throw std::runtime_error("Failed to load variation kernel function");
    }
    // std::cout << "Variation kernel loaded" << std::endl;

    // std::cout << "Loading reduce kernel..." << std::endl;
    auto reduceFunction = library->newFunction(NS::String::string("reduce_variation_kernel", NS::UTF8StringEncoding));
    if (!reduceFunction) {
        throw std::runtime_error("Failed to load reduce kernel function");
    }
    // std::cout << "Reduce kernel loaded" << std::endl;

    // Create pipeline states
    // std::cout << "Creating pipeline states..." << std::endl;
    
    // std::cout << "Creating heat equation pipeline state..." << std::endl;
    pipelineState = device->newComputePipelineState(kernelFunction, &error);
    if (!pipelineState) {
        std::string errorMsg = error ? error->localizedDescription()->utf8String() : "Unknown error";
        std::cerr << "Pipeline state creation failed: " << errorMsg << std::endl;
        throw std::runtime_error("Failed to create heat equation pipeline state: " + errorMsg);
    }
    // std::cout << "Heat equation pipeline state created" << std::endl;

    // std::cout << "Creating variation pipeline state..." << std::endl;
    pipelineStateVariation = device->newComputePipelineState(variationFunction, &error);
    if (!pipelineStateVariation) {
        throw std::runtime_error("Failed to create variation pipeline state");
    }
    // std::cout << "Variation pipeline state created" << std::endl;

    // std::cout << "Creating reduce pipeline state..." << std::endl;
    pipelineStateReduce = device->newComputePipelineState(reduceFunction, &error);
    if (!pipelineStateReduce) {
        throw std::runtime_error("Failed to create reduce pipeline state");
    }
    // std::cout << "Reduce pipeline state created" << std::endl;


    // std::cout << "Creating initialization pipeline state..." << std::endl;
    auto initFunction = library->newFunction(NS::String::string("initialize_solution_kernel", NS::UTF8StringEncoding));
    if (!initFunction) {
        throw std::runtime_error("Failed to load initialization kernel function");
    }
    // std::cout << "Initialization kernel loaded" << std::endl;

    pipelineStateInit = device->newComputePipelineState(initFunction, &error);
    if (!pipelineStateInit) {
        std::string errorMsg = error ? error->localizedDescription()->utf8String() : "Unknown error";
        std::cerr << "Init pipeline state creation failed: " << errorMsg << std::endl;
        throw std::runtime_error("Failed to create initialization pipeline state: " + errorMsg);
    }
    // std::cout << "Initialization pipeline state created" << std::endl;

    // std::cout << "=== initializeMetal() completed successfully ===" << std::endl;
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
    // timers("GPU Computation").start();
    // std::cout << "compute one step" << std::endl;
    // timers("Others").start();
    // timers.display();
    // timers("Calculation").start();
    // std::cout << "Computation time start" << std::endl;

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
    
    // timers("GPU Computation").stop();
    // timers("Calculation").stop();
    // std::cout << "Computation time stop" << std::endl;

    
    return total_variation;
}