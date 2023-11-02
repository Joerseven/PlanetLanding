//
// Created by jdhyd on 10/30/2023.
//

#include "Renderer.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent) {
    camera = new Camera();
    //sphere = Mesh::GenerateUVSphere(5, 5);
    quad = Mesh::GenerateQuad();
    cubemap = new Cubemap();

    texture = Texture::LoadTexture(TEXTUREPATH "Barren Reds.JPG");
    shader = new Shader(SHADERPATH "BaseVertex.glsl", SHADERPATH "BaseFragment.glsl");

    if (!shader->LoadSuccess() || !texture) {
        return;
    }

    SetTextureRepeating(texture, true);

    projMatrix = Matrix4::Perspective(0.001f, 15000.0f, (float)width/float(height), 45.0f);
    modelMatrix = Matrix4::Translation(Vector3(0, 0, -10));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    init = true;
}

Renderer::~Renderer() {
    delete sphere;
    delete shader;
    delete quad;
    delete camera;
    delete cubemap;
}

void Renderer::RenderScene() {
    glClearColor(0.1, 0.12, 0.1, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    cubemap->Draw(this);

    BindShader(shader);
    UpdateShaderMatrices();

    shader->SetTexture(0, "diffuseTex", texture);
    //sphere->Draw();
    quad->Draw();


}

void Renderer::UpdateScene(float dt) {
    camera->UpdateCamera(dt);
    viewMatrix = camera->BuildViewMatrix();
}
