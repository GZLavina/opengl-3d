cmake_minimum_required(VERSION 3.27)
project(CGBase)

include_directories(
        Dependencies/include
        Dependencies/${std_image_SOURCE_DIR}
)

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

#FetchContent_Declare(
#        stb_image
#        GIT_REPOSITORY https://github.com/jschmidt42/stb-cmake.git
#)
#FetchContent_GetProperties(stb_image)
#
#if(NOT stb_image_POPULATED)
#    FetchContent_Populate(stb_image)
#endif()

set(CMAKE_CXX_STANDARD 20)

add_executable(CGBase main.cpp
        glad.c
        stb_image.cpp
)

target_link_libraries(CGBase glfw glm::glm)