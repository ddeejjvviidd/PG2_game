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

# Set OpenCV directory based on OS
if(WIN32)  # WIN32 is true on Windows
    set(OpenCV_DIR "C:/opencv/build")  # Your Windows path
endif()
find_package(OpenCV REQUIRED)

# Add executable
add_executable(pg2_project src/main.cpp src/app.cpp src/gl_err_callback.cpp src/ShaderProgram.cpp src/OBJloader.cpp)

# Link libraries (GLM is header-only, so no linking needed)
if(WIN32)
    target_link_libraries(pg2_project PRIVATE glfw GLEW::GLEW ${OpenCV_LIBS} nlohmann_json::nlohmann_json)
elseif(UNIX)
    target_link_libraries(pg2_project PRIVATE glfw GLEW::GLEW OpenGL::GL ${OpenCV_LIBS} nlohmann_json::nlohmann_json)
endif()
