# CMakeLists.txt pour le projet MetalHeat3D
cmake_minimum_required(VERSION 3.20)

# Informations du projet
project(MetalHeat3D VERSION 1.0
                   DESCRIPTION "Projet de simulation de chaleur 3D sur GPU Metal"
                   LANGUAGES CXX)

# Options de configuration du projet
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Chemins des dépendances et ressources
set(METAL_CPP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp")

# Bibliothèque de configuration (header-only)
add_library(config_library INTERFACE)
target_include_directories(config_library INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
)

# Ajouter les sous-projets
add_subdirectory(src/core)
add_subdirectory(src/utils)

# Bibliothèque de métal C++
add_library(metal_cpp INTERFACE)
target_include_directories(metal_cpp INTERFACE 
    ${METAL_CPP_PATH}
)

# Définitions spécifiques à Metal
add_definitions(
    -DNS_PRIVATE_IMPLEMENTATION
    -DCA_PRIVATE_IMPLEMENTATION
    -DMTL_PRIVATE_IMPLEMENTATION
)

# Création de l'exécutable
add_executable(${PROJECT_NAME}
    main.cpp
)

# Définition du chemin de configuration
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/src/config"
)

# Configuration des inclusions pour l'exécutable
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/solvers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Gestion des frameworks
find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
find_library(QUARTZ_FRAMEWORK QuartzCore REQUIRED)

# Lien des bibliothèques
target_link_libraries(${PROJECT_NAME}
    config_library
    core_library
    utils_library
    metal_cpp
    ${METAL_FRAMEWORK}
    ${FOUNDATION_FRAMEWORK}
    ${QUARTZ_FRAMEWORK}
)