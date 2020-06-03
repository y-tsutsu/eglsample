cmake_minimum_required(VERSION 3.0.0)
project(eglsample VERSION 0.1.0)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited configs" FORCE)

add_executable(eglsample
    eglsample.cpp
)

target_compile_features(eglsample
    PRIVATE cxx_std_17
)

find_package(X11 REQUIRED)
find_package(PNG REQUIRED)
find_library(EGL_LIBRARY NAMES EGL)
find_library(GLES2_LIBRARY NAMES GLESv2)

target_link_libraries(eglsample
    ${X11_LIBRARIES}
    ${EGL_LIBRARY}
    ${GLES2_LIBRARY}
    ${PNG_LIBRARIES}
)
