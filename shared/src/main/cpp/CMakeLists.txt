# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

project("shared-texture")

add_library(shared-texture SHARED
        SharedTexture.cpp
        JSharedTexture.cpp)

find_library(android-lib android)
find_library(log-lib log)

target_link_libraries(shared-texture
        GLESv2
        EGL
        jnigraphics
        ${android-lib}
        ${log-lib})
