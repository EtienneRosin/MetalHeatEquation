# CMakeLists.txt pour la bibliothèque utils (header-only)
add_library(config_library INTERFACE)

target_include_directories(config_library INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Si utils dépend d'autres bibliothèques, vous pouvez les lier ici
# target_link_libraries(config_library INTERFACE 
#     config_library
# )