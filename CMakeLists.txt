cmake_minimum_required(VERSION 3.10)

project(TileViewer VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(
    GLFW_USE_SYSTEM_LIB 
    "Uses prebuilt glfw system library instead of compiling from source" 
    ON
)

add_executable(
    tile

    # app sources
    "source/tile/main.cpp"
    "source/tile/gl_wrappers.cpp"
    "source/tile/Window.cpp"
    "source/tile/Shader.cpp"
    "source/tile/Camera.cpp"
    "source/tile/CameraController.cpp"
    "source/tile/Model.cpp"


    # dependencies sources
    "vendor/SLAM/slam/slam.cpp"
    "vendor/tiny_obj_loader/TinyObjLoader/tiny_obj_loader.cpp"
)
target_include_directories(tile PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)
target_include_directories(tile PUBLIC "vendor/SLAM")
# target_include_directories(tile PUBLIC "vendor/GLAD/include")
target_include_directories(tile PUBLIC "vendor/tiny_obj_loader")

find_package(Threads REQUIRED)

if(GLFW_USE_SYSTEM_LIB)
    find_package(PkgConfig REQUIRED)            # Pkg-Config
    pkg_search_module(GLFW REQUIRED glfw3)      # GLFW
else()
    message( FATAL_ERROR "GLFW_USE_SYSTEM_LIB with value of OFF currently not implemented" )
endif()

add_subdirectory(vendor/GLM)

target_link_libraries(
    tile
    
    Threads::Threads
    dl
    ${GLFW_LIBRARIES}
    glm::glm
)

set_property(TARGET tile PROPERTY C_STANDARD 11)
set_property(TARGET tile PROPERTY CXX_STANDARD 17)