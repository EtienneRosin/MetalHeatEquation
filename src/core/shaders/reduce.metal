#include "common.metal"

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