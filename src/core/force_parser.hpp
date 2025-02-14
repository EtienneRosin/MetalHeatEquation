#ifndef FORCE_PARSER_HPP
#define FORCE_PARSER_HPP

#include <string>

class ForceParser {
public:
    struct ParsedForce {
        std::string metalCode;
        std::string originalCode;
    };

    // Parse le fichier force.hpp et retourne le code Metal et le code original
    static ParsedForce parseForceFile(const std::string& filePath);

    // Active/désactive le logging de debug
    static void setDebugMode(bool enable) { debugMode = enable; }

private:
    static bool debugMode;

    // Extrait la fonction f du contenu du fichier
    static std::string extractForceFunction(const std::string& content);
    
    // Convertit le code C++ en code Metal
    static std::string convertToMetalFunction(const std::string& cppCode);
    
    // Vérifie la validité de la fonction extraite
    static void validateForceFunction(const std::string& function);
    
    // Utilitaire de logging pour le debug
    static void debugLog(const std::string& message);
};

#endif