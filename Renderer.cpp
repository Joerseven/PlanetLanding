//
// Created by jdhyd on 10/30/2023.
//

#include "Renderer.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent) {
    camera = new Camera();
    sphere = Mesh::GenerateUVSphere(6, 6);
    quad = Mesh::GenerateQuad();

    texture = Texture::LoadTexture(TEXTUREPATH "Barren Reds.JPG");
    shader = new Shader(SHADERPATH "BaseVertex.glsl", SHADERPATH "BaseFragment.glsl");

    if (!shader->LoadSuccess() || !texture) {
        return;
    }

    SetTextureRepeating(texture, true);

    projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width/float(height), 45.0f);
    modelMatrix = Matrix4::Translation(Vector3(0, 0, -10));
    glEnable(GL_DEPTH_TEST);

    init = true;
}

Renderer::~Renderer() {
    delete sphere;
    delete shader;
    delete quad;
    delete camera;
}

void Renderer::RenderScene() {
    glClearColor(0.1, 0.12, 0.1, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    BindShader(shader);
    UpdateShaderMatrices();

    shader->SetTexture(0, "diffuseTex", texture);

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    sphere->Draw();
    //quad->Draw();

}

void Renderer::UpdateScene(float dt) {
    camera->UpdateCamera(dt);
    viewMatrix = camera->BuildViewMatrix();
}
