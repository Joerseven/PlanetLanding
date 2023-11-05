//
// Created by jdhyd on 10/30/2023.
//

#include "Renderer.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent) {
    camera = new Camera();

    sun = new Model();
    planet = new Model();

    sun->mesh = Mesh::GenerateUVSphere(20, 20);
    planet->mesh = Mesh::GenerateUVSphere(12, 12);

    planet->localTransform = Matrix4::Translation(Vector3(10, 0, -10))
                             * Matrix4::Scale(Vector3(0.5, 0.5, 0.5));

    sun->localTransform = Matrix4::Translation(Vector3(0, 0, -10));

    sun->mesh->SetColor(1.0, 170.0f / 255.0f, 0.0, 1.0);
    planet->mesh->SetColor(0, 0, 82.0f / 255.0f, 1.0);

    sun->shader = new Shader(SHADERPATH "SunVert.glsl", SHADERPATH "SunFrag.glsl");
    planet->shader = new Shader(SHADERPATH "PlanetVert.glsl", SHADERPATH "PlanetFrag.glsl");

    planet->light = light;

    if (!sun->shader->LoadSuccess() || !planet->shader->LoadSuccess()) {
        std::cout << "Fuck" << std::endl;
        return;
    }

    cubemap = new Cubemap();


    light = new Light(Vector3(), Vector4(1,1,1,1), 20);

    projMatrix = Matrix4::Perspective(0.01f, 15000.0f, (float)width/float(height), 45.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    init = true;
}

Renderer::~Renderer() {
    delete sun;
    delete planet;
    delete camera;
    delete light;
}

void Renderer::RenderScene() {
    glClearColor(0.1, 0.12, 0.1, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    cubemap->Draw(this);

    sun->Draw(this);
    planet->Draw(this);

}

void Renderer::UpdateScene(float dt) {
    camera->UpdateCamera(dt);
    viewMatrix = camera->BuildViewMatrix();
}

void Model::Draw(Renderer *context) {
    context->BindShader(shader);
    context->UpdateShaderMatrices();

//    if (light) {
//        glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)&context->camera->Position);
//        context->SetShaderLight(*light);
//    }

    glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, localTransform.values);
    mesh->Draw();
}
