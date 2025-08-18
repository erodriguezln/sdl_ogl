cmake_minimum_required(VERSION 3.30)
project(sdl_ogl)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
include_directories(glad/include)
include_directories(glm)

# Find SDL3 and SDL3_image using CMake's find_package
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

# Include SDL3 and SDL3_image headers
include_directories(${SDL3_INCLUDE_DIRS} ${SDL3_IMAGE_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES "src/*.cpp")

# Add GLAD source file
set(GLAD_SOURCES glad/src/glad.c)

# Add your source files including GLAD
add_executable(${PROJECT_NAME} ${SOURCES} ${GLAD_SOURCES})

# Link against SDL3 and SDL3_image libraries
target_link_libraries(${PROJECT_NAME} SDL3::SDL3 SDL3_image::SDL3_image)

# Detectar archivos de shaders
file(GLOB SHADER_FILES
        "${CMAKE_SOURCE_DIR}/shaders/*.vert"
        "${CMAKE_SOURCE_DIR}/shaders/*.frag"
)

# Crear target personalizado para copiar shaders cuando cambien
add_custom_target(copy-shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
        DEPENDS ${SHADER_FILES}
        COMMENT "Copying shaders to build directory"
)

# Copiar assets en POST_BUILD
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying assets to build directory"
)

# Hacer que el ejecutable dependa de copy-shaders
add_dependencies(${PROJECT_NAME} copy-shaders)

# Mensaje de información útil
message(STATUS "Shaders will be copied from: ${CMAKE_SOURCE_DIR}/shaders")
message(STATUS "Shaders will be copied to: $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders")