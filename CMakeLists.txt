# this is just code from gemini man, i have no idea whats going on here

cmake_minimum_required(VERSION 3.15)
project(ParticleSimulation)

# Use C++ 17
set(CMAKE_CXX_STANDARD 17)

# 1. Download GLFW automatically
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 2. Find the system's OpenGL library
find_package(OpenGL REQUIRED)

# 3. Create our executable using main.cpp and glad.c
add_executable(ParticleSim main.cpp src/glad.c)

# 4. Tell the compiler where our header files are
target_include_directories(ParticleSim PRIVATE include)

# 5. Link GLFW and OpenGL to our executable
target_link_libraries(ParticleSim PRIVATE glfw OpenGL::GL)

# help find the shaders
# file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/include/Shaders" 
#     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")