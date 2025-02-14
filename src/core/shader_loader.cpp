#include "shader_loader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::string ShaderLoader::readShaderFile(const std::string& filename) {
    std::ifstream file("../src/core/shaders/" + filename);
    if (!file) {
        throw std::runtime_error("Cannot open shader file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// std::string ShaderLoader::combineShaders(const std::vector<std::string>& shaderContents,
//                                        const std::string& forceFunction,
//                                        const std::string& initialCondition) {
//     std::string combinedShader;
    
//     // Ajouter le contenu de common.metal en premier
//     combinedShader = shaderContents[0];
    
//     // Remplacer la déclaration de la fonction force
//     // size_t forcePos = combinedShader.find("[[visible]] METAL_FUNC float f(float x, float y, float z, float t);");
//     size_t forcePos = combinedShader.find("METAL_FUNC float f(float x, float y, float z, float t);");
//     if (forcePos != std::string::npos) {
//         size_t lineStart = combinedShader.rfind('\n', forcePos);
//         if (lineStart == std::string::npos) lineStart = 0;
//         combinedShader = combinedShader.substr(0, lineStart) + "\n" + forceFunction;
//     }
    
//     // Remplacer la déclaration de la fonction de condition initiale
//     // size_t initPos = combinedShader.find("[[visible]] METAL_FUNC float g(float x, float y, float z);");
//     size_t initPos = combinedShader.find("METAL_FUNC float g(float x, float y, float z);");
//     if (initPos != std::string::npos) {
//         size_t lineStart = combinedShader.rfind('\n', initPos);
//         if (lineStart == std::string::npos) lineStart = 0;
//         combinedShader = combinedShader.substr(0, lineStart) + "\n" + initialCondition;
//     }
    
//     // Ajouter les autres shaders
//     for (size_t i = 1; i < shaderContents.size(); ++i) {
//         std::string shaderContent = shaderContents[i];
//         size_t includePos = shaderContent.find("#include \"common.metal\"");
//         if (includePos != std::string::npos) {
//             size_t endPos = shaderContent.find('\n', includePos);
//             shaderContent = shaderContent.substr(endPos + 1);
//         }
//         combinedShader += "\n" + shaderContent;
//     }
    
//     return combinedShader;
// }

std::string ShaderLoader::combineShaders(const std::vector<std::string>& shaderContents,
        const std::string& forceFunction,
        const std::string& initialCondition) {
    std::string combinedShader = shaderContents[0];

    // Expressions régulières plus flexibles pour trouver les déclarations
    std::regex forceRegex(R"((?:\[\[visible\]\]\s+)?METAL_FUNC\s+float\s+f\s*\(\s*float\s+x\s*,\s*float\s+y\s*,\s*float\s+z\s*,\s*float\s+t\s*\)\s*;)");
    std::regex initRegex(R"((?:\[\[visible\]\]\s+)?METAL_FUNC\s+float\s+g\s*\(\s*float\s+x\s*,\s*float\s+y\s*,\s*float\s+z\s*\)\s*;)");

    // Remplacer les déclarations
    combinedShader = std::regex_replace(combinedShader, forceRegex, forceFunction);
    combinedShader = std::regex_replace(combinedShader, initRegex, initialCondition);

    // Ajouter les autres shaders
    for (size_t i = 1; i < shaderContents.size(); ++i) {
    std::string shaderContent = shaderContents[i];
    size_t includePos = shaderContent.find("#include \"common.metal\"");
    if (includePos != std::string::npos) {
    size_t endPos = shaderContent.find('\n', includePos);
    shaderContent = shaderContent.substr(endPos + 1);
    }
    combinedShader += "\n" + shaderContent;
    }

    return combinedShader;
}



// std::string ShaderLoader::combineShaders(const std::vector<std::string>& shaderContents,
//                                        const std::string& forceFunction,
//                                        const std::string& initialCondition) {
//     std::string combinedShader;
    
//     // Commencer par les déclarations des fonctions f et g
//     combinedShader = forceFunction + "\n" + initialCondition + "\n\n";
    
//     // Ajouter le contenu de common.metal
//     combinedShader += shaderContents[0];
    
//     // Supprimer les déclarations originales de f et g si elles existent
//     size_t forcePos = combinedShader.find("[[visible]] METAL_FUNC float f(float x, float y, float z, float t);");
//     if (forcePos != std::string::npos) {
//         size_t lineEnd = combinedShader.find('\n', forcePos);
//         if (lineEnd != std::string::npos) {
//             combinedShader.erase(forcePos, lineEnd - forcePos + 1);
//         }
//     }
    
//     size_t initPos = combinedShader.find("[[visible]] METAL_FUNC float g(float x, float y, float z);");
//     if (initPos != std::string::npos) {
//         size_t lineEnd = combinedShader.find('\n', initPos);
//         if (lineEnd != std::string::npos) {
//             combinedShader.erase(initPos, lineEnd - initPos + 1);
//         }
//     }
    
//     // Ajouter les autres shaders
//     for (size_t i = 1; i < shaderContents.size(); ++i) {
//         std::string shaderContent = shaderContents[i];
//         size_t includePos = shaderContent.find("#include \"common.metal\"");
//         if (includePos != std::string::npos) {
//             size_t endPos = shaderContent.find('\n', includePos);
//             shaderContent = shaderContent.substr(endPos + 1);
//         }
//         combinedShader += "\n" + shaderContent;
//     }
    
//     return combinedShader;
// }


std::string ShaderLoader::loadShaders(const std::string& forceFunction, const std::string& initialCondition) {
    std::vector<std::string> shaderContents;
    
    const std::vector<std::string> shaderFiles = {
        "common.metal",
        "heat_equation.metal",
        "variation.metal",
        "reduce.metal",
        "initialization.metal"
    };
    
    for (const auto& file : shaderFiles) {
        shaderContents.push_back(readShaderFile(file));
    }
    
    return combineShaders(shaderContents, forceFunction, initialCondition);
}