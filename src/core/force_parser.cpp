#include "force_parser.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <iostream>

bool ForceParser::debugMode = false;

void ForceParser::debugLog(const std::string& message) {
    if (debugMode) {
        std::cout << "[ForceParser Debug] " << message << std::endl;
    }
}

ForceParser::ParsedForce ForceParser::parseForceFile(const std::string& filePath) {
    debugLog("Parsing file: " + filePath);
    
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Cannot open force file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    debugLog("File content:\n" + content);
    
    std::string originalFunction = extractForceFunction(content);
    debugLog("Extracted original function:\n" + originalFunction);
    
    validateForceFunction(originalFunction);
    debugLog("Function validation passed");
    
    std::string metalFunction = convertToMetalFunction(originalFunction);
    debugLog("Converted Metal function:\n" + metalFunction);
    
    return ParsedForce{metalFunction, originalFunction};
}

std::string ForceParser::extractForceFunction(const std::string& content) {
    size_t start = content.find("inline double f");
    size_t end = content.find("#endif");
    
    if (start == std::string::npos || end == std::string::npos) {
        throw std::runtime_error("Cannot find force function in the file");
    }
    
    debugLog("Found function bounds: start=" + std::to_string(start) + ", end=" + std::to_string(end));
    return content.substr(start, end - start);
}

void ForceParser::validateForceFunction(const std::string& function) {
    debugLog("Validating function...");
    
    bool hasX = function.find("double x") != std::string::npos;
    bool hasY = function.find("double y") != std::string::npos;
    bool hasZ = function.find("double z") != std::string::npos;
    bool hasT = function.find("double t") != std::string::npos;
    
    debugLog("Parameters found: x=" + std::to_string(hasX) + 
             ", y=" + std::to_string(hasY) + 
             ", z=" + std::to_string(hasZ) + 
             ", t=" + std::to_string(hasT));
    
    if (!hasX || !hasY || !hasZ || !hasT) {
        throw std::runtime_error("Force function must have parameters (double x, double y, double z, double t)");
    }
    
    bool hasReturn = function.find("return") != std::string::npos;
    debugLog("Return statement found: " + std::to_string(hasReturn));
    
    if (!hasReturn) {
        throw std::runtime_error("Force function must have a return statement");
    }
}

std::string ForceParser::convertToMetalFunction(const std::string& cppCode) {
    debugLog("Starting Metal conversion");
    std::string metalCode = cppCode;
    
    // Remplacement de inline double par METAL_FUNC float
    std::regex inlineDoubleRegex(R"(\binline\s+double\b)");
    metalCode = std::regex_replace(metalCode, inlineDoubleRegex, "METAL_FUNC float");
    // std::regex inlineDoubleRegex(R"(\binline\s+double\b)");
    // metalCode = std::regex_replace(metalCode, inlineDoubleRegex, "[[visible]] METAL_FUNC float");

    debugLog("After inline double replacement:\n" + metalCode);
    
    // Remplacement de tous les double par float
    std::regex doubleRegex(R"(\bdouble\b)");
    metalCode = std::regex_replace(metalCode, doubleRegex, "float");
    debugLog("After double replacement:\n" + metalCode);
    
    // Ajout du suffixe f aux constantes numériques (amélioré)
    // Cette regex capture les nombres littéraux mais pas les variables
    std::regex numberRegex(R"((-?\b\d*\.?\d+\b)(?![a-zA-Z_]))");
    metalCode = std::regex_replace(metalCode, numberRegex, "$1f");
    debugLog("After number conversion:\n" + metalCode);
    
    return metalCode;
}