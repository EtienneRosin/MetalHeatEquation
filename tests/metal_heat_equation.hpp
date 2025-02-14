#ifndef METAL_HEAT_EQUATION_HPP
#define METAL_HEAT_EQUATION_HPP

#include "heat_equation.hpp"
#include "force_parser.hpp"
#include <Metal/Metal.hpp>
#include <functional>

class MetalHeatEquation : public HeatEquation {
private:
    // Metal resources
    MTL::Device* device;
    MTL::CommandQueue* commandQueue;
    MTL::Library* library;
    MTL::Function* kernelFunction;
    MTL::ComputePipelineState* pipelineState;
    MTL::ComputePipelineState* pipelineStateVariation;  // Nouveau
    MTL::ComputePipelineState* pipelineStateReduce;     // Nouveau
    MTL::Buffer* debugBuffer;
    
    // Buffers
    MTL::Buffer* currentBuffer;
    MTL::Buffer* nextBuffer;
    MTL::Buffer* paramsBuffer;
    MTL::Buffer* variationBuffer;  // Nouveau
    MTL::Buffer* resultBuffer;     // Nouveau
    
    void initializeMetal();
    double compute_timestep() override;
    void setupBuffers();

public:
    MetalHeatEquation(Parameters params, 
                      std::function<double(double,double,double,double)> f,
                      std::function<double(double,double,double)> g);
    ~MetalHeatEquation();
};

#endif