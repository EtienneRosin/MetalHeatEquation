# Création de la bibliothèque core
add_library(core_library STATIC
    solution.cpp
    heat_equation.cpp
    metal_heat_equation.cpp
    force_parser.cpp
    shader_loader.cpp
)

# Définition du chemin de configuration
target_compile_definitions(core_library PRIVATE
    CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../config"
)

# Configuration des inclusions
target_include_directories(core_library PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
)

# Lien avec les autres bibliothèques
target_link_libraries(core_library
    config_library
    utils_library
    metal_cpp
    ${METAL_FRAMEWORK}
    ${FOUNDATION_FRAMEWORK}
    ${QUARTZ_FRAMEWORK}
)