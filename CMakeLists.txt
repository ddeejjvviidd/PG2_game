cmake_minimum_required(VERSION 3.10)
project(pg2_project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Find OpenCV manually from C:\opencv
# IMPORTANT! Davide, zakomentuj prosím následující řádek!
set(OpenCV_DIR "C:/opencv/build")  # Point to the build directory
find_package(OpenCV REQUIRED)

# Add executable
add_executable(pg2_project src/main.cpp src/app.cpp src/gl_err_callback.cpp)

# Link libraries (GLM is header-only, so no linking needed)
target_link_libraries(pg2_project PRIVATE glfw GLEW::GLEW ${OpenCV_LIBS} nlohmann_json::nlohmann_json)
