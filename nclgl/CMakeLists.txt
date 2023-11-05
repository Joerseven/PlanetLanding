cmake_minimum_required(VERSION 3.1...3.27)
project(nclgl)

FIND_PACKAGE(OpenGL)

add_subdirectory(glad)

add_library(nclgl
        ComputeShader.cpp
        Camera.cpp
        GameTimer.cpp
        Keyboard.cpp
        Matrix2.cpp
        Matrix3.cpp
        Matrix4.cpp
        Mesh.cpp
        MeshAnimation.cpp
        MeshMaterial.cpp
        Mouse.cpp
        OGLRenderer.cpp
        Quaternion.cpp
        Shader.cpp
        Window.cpp
        SceneNode.cpp
        HeightMap.cpp
        Texture.cpp
        Light.cpp
        Cubemap.cpp
        PostPass.cpp
        Component.cpp
)


target_include_directories(nclgl PUBLIC .)
target_link_libraries(nclgl PUBLIC glad ${OPENGL_gl_LIBRARY})