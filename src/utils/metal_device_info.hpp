/**
 * @file MetalDeviceInfo.hpp
 * @brief Class to analyze and display Metal GPU devices information
 * 
 * This class provides methods to:
 * 1. List all available Metal devices
 * 2. Get detailed information about each device
 * 3. Format and display this information
 * 
 * Device Information Details:
 * 
 * Basic Information:
 * - Name: Model and identification of the GPU
 * - Registry ID: Unique system identifier for the device
 * - Architecture: Hardware architecture name and version
 * 
 * Power and Type Characteristics:
 * - Low Power Mode: Indicates if the GPU is designed for power efficiency
 *   * Yes: Optimized for battery life, might have lower performance
 *   * No: Full performance mode, typically higher power consumption
 * 
 * - Headless: Indicates if the GPU has display capabilities
 *   * Yes: Compute-only GPU, optimal for computational tasks
 *   * No: GPU can output to displays
 * 
 * - Removable: Indicates if the GPU is external/detachable
 *   * Yes: External GPU (eGPU)
 *   * No: Internal GPU
 * 
 * Memory Specifications:
 * - Unified Memory: Indicates shared memory architecture between CPU and GPU
 *   * Yes: Memory is shared, faster CPU-GPU transfers
 *   * No: Separate memory pools, explicit transfers needed
 * 
 * - Recommended Max Working Set Size: Optimal memory allocation size
 *   * Indicates the ideal maximum amount of memory to allocate
 *   * Exceeding this may impact performance
 * 
 * - Max Buffer Length: Maximum size of a single buffer
 *   * Critical for large data structures
 *   * Limits maximum size of arrays/textures
 * 
 * Advanced Capabilities:
 * - Ray Tracing Support: Hardware acceleration for ray tracing
 *   * Useful for advanced graphics and scientific simulations
 * 
 * - Dynamic Libraries Support: Runtime code loading capability
 *   * Enables dynamic shader loading and compilation
 * 
 * Thread Configuration:
 * - Max Threads Per Threadgroup: Maximum thread dimensions
 *   * Width: X dimension maximum threads
 *   * Height: Y dimension maximum threads
 *   * Depth: Z dimension maximum threads
 *   * Important for optimizing compute kernel dispatch
 * 
 * Location Information:
 * - Location: Physical location/type of the GPU
 *   * Built-in: Integrated into the system
 *   * Slot: PCIe or similar slot
 *   * External: External connection (Thunderbolt, etc.)
 *   * Unspecified: Location unknown
 * 
 * - Location Number: Specific slot or port number
 *   * Useful for systems with multiple GPUs
 * 
 * Peer Information:
 * - Peer Group ID: Group identifier for related GPUs
 * - Peer Index: Position within the peer group
 * - Peer Count: Total number of GPUs in the group
 *   * Important for multi-GPU configurations
 * 
 * Usage example:
 * @code
 * MetalDeviceInfo deviceInfo;
 * deviceInfo.displayAllDevicesInfo();
 * // Or get specific device info
 * auto device = deviceInfo.getDevice(0);
 * deviceInfo.displayDeviceInfo(device);
 * @endcode
 * 
 * Performance Considerations:
 * 1. For compute tasks, prefer:
 *    - Headless GPUs
 *    - Non-low power devices
 *    - Devices with unified memory (for data-intensive tasks)
 * 
 * 2. For memory-intensive operations:
 *    - Check Max Buffer Length
 *    - Consider Unified Memory support
 *    - Stay within Recommended Max Working Set Size
 * 
 * 3. For parallel processing:
 *    - Use Max Threads Per Threadgroup for optimal grid sizing
 *    - Consider multi-GPU setup using peer information
 */

#pragma once
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

class MetalDeviceInfo {
public:
    /**
     * @brief Construct a new Metal Device Info object
     * Initializes the devices list
     */
    MetalDeviceInfo() {
        devices = MTL::CopyAllDevices();
    }

    /**
     * @brief Destroy the Metal Device Info object
     * Cleanup allocated resources
     */
    ~MetalDeviceInfo() {
        if (devices) {
            devices->release();
        }
    }

    /**
     * @brief Get the number of available Metal devices
     * @return uint32_t Number of devices
     */
    uint32_t getDeviceCount() const {
        return devices ? devices->count() : 0;
    }

    /**
     * @brief Get a specific device by index
     * @param index Device index
     * @return MTL::Device* Pointer to the device, nullptr if invalid index
     */
    MTL::Device* getDevice(uint32_t index) const {
        if (!devices || index >= devices->count()) {
            return nullptr;
        }
        return static_cast<MTL::Device*>(devices->object(index));
    }

    /**
     * @brief Convert device location to string
     * @param location Device location enum
     * @return std::string Human-readable location
     */
    static std::string locationToString(MTL::DeviceLocation location) {
        switch(location) {
            case MTL::DeviceLocationBuiltIn: return "Built-in";
            case MTL::DeviceLocationSlot: return "Slot";
            case MTL::DeviceLocationExternal: return "External";
            case MTL::DeviceLocationUnspecified: return "Unspecified";
            default: return "Unknown";
        }
    }

    // /**
    //  * @brief Display detailed information about a specific device
    //  * @param device Pointer to the Metal device
    //  * @param index Optional index for display purposes
    //  */
    // void displayDeviceInfo(MTL::Device* device, uint32_t index = 0) const {
    //     if (!device) return;

    //     std::cout << "\nGPU Device " << index + 1 << ":" << std::endl;
        
    //     // Basic information
    //     std::cout << "  Name: " << device->name()->utf8String() << std::endl;
    //     std::cout << "  Registry ID: " << device->registryID() << std::endl;
    //     std::cout << "  Architecture: " << device->architecture()->name()->utf8String() << std::endl;
        
    //     // Power and type characteristics
    //     std::cout << "  Low Power Mode: " << (device->lowPower() ? "Yes" : "No") << std::endl;
    //     std::cout << "  Headless: " << (device->headless() ? "Yes" : "No") << std::endl;
    //     std::cout << "  Removable: " << (device->removable() ? "Yes" : "No") << std::endl;
        
    //     // Memory information
    //     std::cout << "  Unified Memory: " << (device->hasUnifiedMemory() ? "Yes" : "No") << std::endl;
    //     std::cout << "  Recommended Max Working Set Size: " 
    //             << (device->recommendedMaxWorkingSetSize() / (1024*1024)) << " MB" << std::endl;
    //     std::cout << "  Max Buffer Length: " 
    //             << (device->maxBufferLength() / (1024*1024)) << " MB" << std::endl;
        
    //     // Capabilities
    //     std::cout << "  Supports Ray Tracing: " << (device->supportsRaytracing() ? "Yes" : "No") << std::endl;
    //     std::cout << "  Supports Dynamic Libraries: " << (device->supportsDynamicLibraries() ? "Yes" : "No") << std::endl;
        
    //     // Thread information
    //     MTL::Size maxThreads = device->maxThreadsPerThreadgroup();
    //     std::cout << "  Max Threads Per Threadgroup:" << std::endl;
    //     std::cout << "    Width: " << maxThreads.width << std::endl;
    //     std::cout << "    Height: " << maxThreads.height << std::endl;
    //     std::cout << "    Depth: " << maxThreads.depth << std::endl;
        
    //     // Location information
    //     std::cout << "  Location: " << locationToString(device->location()) << std::endl;
    //     std::cout << "  Location Number: " << device->locationNumber() << std::endl;
        
    //     // Peer grouping information
    //     std::cout << "  Peer Group ID: " << device->peerGroupID() << std::endl;
    //     std::cout << "  Peer Index: " << device->peerIndex() << std::endl;
    //     std::cout << "  Peer Count: " << device->peerCount() << std::endl;
        
    //     std::cout << "------------------" << std::endl;
    // }

    // /**
    //  * @brief Display information about all available Metal devices
    //  */
    // void displayAllDevicesInfo() const {
    //     std::cout << "Number of Metal devices found: " << getDeviceCount() << std::endl;
        
    //     for (uint32_t i = 0; i < getDeviceCount(); i++) {
    //         MTL::Device* device = getDevice(i);
    //         if (device) {
    //             displayDeviceInfo(device, i);
    //         }
    //     }
    // }
    /**
     * @brief Get device information as a formatted string
     * @param device Pointer to the Metal device
     * @param index Optional index for display purposes
     * @return std::string Formatted device information
     */
    std::string getDeviceInfoString(MTL::Device* device, uint32_t index = 0) const {
        if (!device) return "";
        
        std::stringstream ss;
        
        ss << "\n╔══════════════════════════════════════════════════════════════╗\n";
        ss << "║                     GPU Device " << index + 1 << "                             ║\n";
        ss << "╠══════════════════════════════════════════════════════════════╣\n";
        
        // Basic information
        ss << "║ Basic Information:                                           ║\n";
        ss << "║   Name: " << device->name()->utf8String() << "\n";
        ss << "║   Registry ID: " << device->registryID() << "\n";
        ss << "║   Architecture: " << device->architecture()->name()->utf8String() << "\n";
        
        ss << "╠──────────────────────────────────────────────────────────────╣\n";
        
        // Power and type characteristics
        ss << "║ Power and Type:                                              ║\n";
        ss << "║   Low Power Mode: " << (device->lowPower() ? "Yes" : "No") << "\n";
        ss << "║   Headless: " << (device->headless() ? "Yes" : "No") << "\n";
        ss << "║   Removable: " << (device->removable() ? "Yes" : "No") << "\n";
        
        ss << "╠──────────────────────────────────────────────────────────────╣\n";
        
        // Memory information
        ss << "║ Memory:                                                      ║\n";
        ss << "║   Unified Memory: " << (device->hasUnifiedMemory() ? "Yes" : "No") << "\n";
        ss << "║   Max Working Set: " << (device->recommendedMaxWorkingSetSize() / (1024*1024)) << " MB\n";
        ss << "║   Max Buffer Length: " << (device->maxBufferLength() / (1024*1024)) << " MB\n";
        
        ss << "╠──────────────────────────────────────────────────────────────╣\n";
        
        // Thread information
        MTL::Size maxThreads = device->maxThreadsPerThreadgroup();
        ss << "║ Thread Configuration:                                        ║\n";
        ss << "║   Max Threads Per Threadgroup:                               ║\n";
        ss << "║     Width: " << maxThreads.width << "\n";
        ss << "║     Height: " << maxThreads.height << "\n";
        ss << "║     Depth: " << maxThreads.depth << "\n";
        
        ss << "╚══════════════════════════════════════════════════════════════╝\n";
        
        return ss.str();
    }

    /**
     * @brief Get all devices information as a string
     * @return std::string Formatted information about all devices
     */
    std::string getAllDevicesInfoString() const {
        std::stringstream ss;
        
        ss << "\n╔══════════════════════════════════════════════════════════════╗\n";
        ss << "║                   Metal Devices Summary                      ║\n";
        ss << "╠══════════════════════════════════════════════════════════════╣\n";
        ss << "║ Number of Metal devices found: " << getDeviceCount() << "                             ║";
        
        for (uint32_t i = 0; i < getDeviceCount(); i++) {
            MTL::Device* device = getDevice(i);
            if (device) {
                ss << getDeviceInfoString(device, i);
            }
        }
        
        return ss.str();
    }

    /**
     * @brief Display information about all available Metal devices
     */
    void displayAllDevicesInfo() const {
        std::cout << getAllDevicesInfoString() << std::endl;
    }
private:
    NS::Array* devices;  ///< Array of available Metal devices
};