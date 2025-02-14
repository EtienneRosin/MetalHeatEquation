#ifndef SHADER_LOADER_HPP
#define SHADER_LOADER_HPP

#include <string>
#include <vector>
#include <regex>

class ShaderLoader {
public:
    // Charge et combine tous les shaders nécessaires
    static std::string loadShaders(const std::string& forceFunction, const std::string& initialCondition);

private:
    // Lit le contenu d'un fichier shader
    static std::string readShaderFile(const std::string& filename);
    
    // Combine tous les shaders avec les fonctions
    static std::string combineShaders(const std::vector<std::string>& shaderContents,
                                    const std::string& forceFunction,
                                    const std::string& initialCondition);
};

#endif