cmake_minimum_required(VERSION 2.8)
add_definitions("-Wall -std=c++11")
# add_definitions("-DLOAD_PNG")
set(CMAKE_BUILD_TYPE Debug)
add_executable(eglsample
    eglsample.cpp
)
find_package(X11 REQUIRED)
find_library(EGL_LIBRARY NAMES EGL)
find_library(GLES2_LIBRARY NAMES GLESv2)
find_package(PNG REQUIRED)
target_link_libraries(eglsample
    ${X11_LIBRARIES}
    ${EGL_LIBRARY}
    ${GLES2_LIBRARY}
    ${PNG_LIBRARIES}
)
