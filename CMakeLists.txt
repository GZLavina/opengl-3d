cmake_minimum_required(VERSION 3.27)
project(CGBase)

include_directories(Dependencies/include)

include(FetchContent)
FetchContent_Declare(
        GLFW
        GIT_REPOSITORY https://github.com/glfw/glfw.git)
FetchContent_MakeAvailable(GLFW)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

set(CMAKE_CXX_STANDARD 20)

add_executable(CGBase main.cpp
        glad.c)

target_link_libraries(CGBase glfw glm::glm)