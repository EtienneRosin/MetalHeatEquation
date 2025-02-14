#ifndef FUNCTION_PARSER_HPP
#define FUNCTION_PARSER_HPP

#include <string>
#include <vector>
#include <regex>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

class FunctionParser {
public:
    struct Parameter {
        std::string type;
        std::string name;
    };

    struct FunctionSignature {
        std::string name;
        std::string returnType;
        std::vector<Parameter> parameters;
    };

    struct ParsedFunction {
        std::string metalCode;
        std::string originalCode;
        FunctionSignature signature;
    };

    // Options de configuration pour le parseur
    struct ParserOptions {
        std::string functionName;           // Nom de la fonction à chercher
        std::vector<std::string> requiredParams;  // Paramètres requis (types)
        bool requireInline = true;          // La fonction doit-elle être inline?
        bool debugMode = false;             // Mode debug
    };

    static ParsedFunction parseFile(const std::string& filePath, const ParserOptions& options) {
        if (options.debugMode) {
            debugLog("Parsing file: " + filePath);
        }

        // Lecture du fichier
        std::ifstream file(filePath);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filePath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // Extraction et validation de la fonction
        std::string originalFunction = extractFunction(content, options);
        FunctionSignature signature = parseFunctionSignature(originalFunction);
        validateFunction(signature, options);
        
        // Conversion en Metal
        std::string metalFunction = convertToMetalFunction(originalFunction);

        return ParsedFunction{metalFunction, originalFunction, signature};
    }

private:
    static void debugLog(const std::string& message) {
        std::cout << "[FunctionParser Debug] " << message << std::endl;
    }

    static std::string extractFunction(const std::string& content, const ParserOptions& options) {
        // Construction du pattern regex pour trouver la fonction
        std::string pattern = options.requireInline ? 
            R"(\binline\s+double\s+)" + options.functionName + R"(\s*\([^)]*\)[^;{]*\{[^}]*\})" :
            R"(\bdouble\s+)" + options.functionName + R"(\s*\([^)]*\)[^;{]*\{[^}]*\})";

        std::regex functionRegex(pattern);
        std::smatch matches;

        if (std::regex_search(content, matches, functionRegex)) {
            return matches[0];
        }

        throw std::runtime_error("Cannot find function " + options.functionName + " in the file");
    }

    static FunctionSignature parseFunctionSignature(const std::string& function) {
        FunctionSignature sig;
        
        // Pattern pour extraire la signature de la fonction
        std::regex signatureRegex(R"((?:inline\s+)?(\w+)\s+(\w+)\s*\((.*?)\))");
        std::smatch matches;
        
        if (std::regex_search(function, matches, signatureRegex)) {
            sig.returnType = matches[1];
            sig.name = matches[2];
            
            // Parse des paramètres
            std::string params = matches[3];
            std::regex paramRegex(R"((\w+)\s+(\w+)(?:\s*,\s*)?)");
            std::sregex_iterator iter(params.begin(), params.end(), paramRegex);
            std::sregex_iterator end;
            
            while (iter != end) {
                Parameter param;
                param.type = (*iter)[1];
                param.name = (*iter)[2];
                sig.parameters.push_back(param);
                ++iter;
            }
        }
        
        return sig;
    }

    static void validateFunction(const FunctionSignature& signature, const ParserOptions& options) {
        // Vérification des paramètres requis
        if (signature.parameters.size() != options.requiredParams.size()) {
            throw std::runtime_error("Function must have exactly " + 
                                   std::to_string(options.requiredParams.size()) + 
                                   " parameters");
        }

        for (size_t i = 0; i < options.requiredParams.size(); ++i) {
            if (signature.parameters[i].type != options.requiredParams[i]) {
                throw std::runtime_error("Parameter " + std::to_string(i) + 
                                       " must be of type " + options.requiredParams[i]);
            }
        }
    }

    // static std::string convertToMetalFunction(const std::string& cppCode) {
    //     std::string metalCode = cppCode;
        
    //     // Remplacement de inline double par METAL_FUNC float
    //     std::regex inlineDoubleRegex(R"(\binline\s+double\b)");
    //     metalCode = std::regex_replace(metalCode, inlineDoubleRegex, "METAL_FUNC float");
        
    //     // Remplacement de double par float
    //     std::regex doubleRegex(R"(\bdouble\b)");
    //     metalCode = std::regex_replace(metalCode, doubleRegex, "float");
        
    //     // Ajout du suffixe f aux constantes numériques
    //     std::regex numberRegex(R"((-?\b\d*\.?\d+\b)(?![a-zA-Z_]))");
    //     metalCode = std::regex_replace(metalCode, numberRegex, "$1f");
        
    //     return metalCode;
    // }


    static std::string convertToMetalFunction(const std::string& cppCode) {
        std::string metalCode = cppCode;
        
        // Remplacement de inline double par METAL_FUNC float
        std::regex inlineDoubleRegex(R"(\binline\s+double\b)");
        metalCode = std::regex_replace(metalCode, inlineDoubleRegex, "METAL_FUNC float");
        
        // Remplacement de double par float
        std::regex doubleRegex(R"(\bdouble\b)");
        metalCode = std::regex_replace(metalCode, doubleRegex, "float");
        
        // Ajout du suffixe f aux constantes numériques
        std::regex numberRegex(R"((-?\b\d*\.?\d+\b)(?![a-zA-Z_]))");
        metalCode = std::regex_replace(metalCode, numberRegex, "$1f");
        
        // Ajouter le namespace metal:: pour les fonctions mathématiques
        std::vector<std::string> mathFunctions = {"sin", "cos", "exp", "pow", "sqrt", "log", "abs"};
        for (const auto& func : mathFunctions) {
            std::regex funcRegex("\\b" + func + "\\b(?=\\s*\\()");
            metalCode = std::regex_replace(metalCode, funcRegex, "metal::" + func);
        }
        
        return metalCode;
    }


};

#endif