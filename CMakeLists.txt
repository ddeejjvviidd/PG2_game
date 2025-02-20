cmake_minimum_required(VERSION 3.10)
project(pg2_project)

# Nastavení C++ standardu
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find packages in system
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# main file
add_executable(pg2_project src/main.cpp)

# link GLFW
target_link_libraries(pg2_project PRIVATE glfw GLEW::GLEW ${OpenCV_LIBS} nlohmann_json::nlohmann_json)

