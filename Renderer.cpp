//
// Created by jdhyd on 10/30/2023.
//

#include "Renderer.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent) {
    camera = new Camera();
    quad = Mesh::GenerateQuad();

    texture = Texture::LoadTexture(TEXTUREPATH "Barren Reds.JPG");
    shader = new Shader(SHADERPATH "BaseVertex.glsl", SHADERPATH "BaseFragment.glsl");

    if (!shader->LoadSuccess() || !texture) {
        return;
    }

    SetTextureRepeating(texture, true);

    projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width/float(height), 45.0f);
    glEnable(GL_DEPTH_TEST);

    init = true;
}

Renderer::~Renderer() {
    delete quad;
    delete shader;
    delete camera;
}

void Renderer::RenderScene() {
    glClearColor(1.0, 0.5, 0.2, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    BindShader(shader);
    UpdateShaderMatrices();

    shader->SetTexture(0, "diffuseTex", texture);

    quad->Draw();

}

void Renderer::UpdateScene(float dt) {
    camera->UpdateCamera(dt);
    viewMatrix = camera->BuildViewMatrix();
}
