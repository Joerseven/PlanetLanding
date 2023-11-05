//
// Created by jdhyd on 10/30/2023.
//

#include "Renderer.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent) {
    camera = new Camera();

    sun = new Model();
    planet = new Model();
    planet2 = new Model();
    planet3 = new Model();

    sun->mesh = Mesh::GenerateUVSphere(20, 20);
    planet->mesh = Mesh::GenerateUVSphere(18, 18);
    planet2->mesh = Mesh::GenerateUVSphere(18, 18);
    planet3->mesh = Mesh::GenerateUVSphere(18, 18);

    planet->localTransform = Matrix4::Rotation(0, Vector3(0, 1, 0))
                             * Matrix4::Translation(Vector3(10, 0, 0))
                             * Matrix4::Scale(Vector3(0.5, 0.5, 0.5));
    planet2->localTransform = Matrix4::Rotation(-60, Vector3(0, 1, 0))
                             * Matrix4::Translation(Vector3(20, 0, 0))
                             * Matrix4::Scale(Vector3(0.8, 0.8, 0.8));
    planet3->localTransform = Matrix4::Rotation(150, Vector3(0, 1, 0))
                             * Matrix4::Translation(Vector3(30, 0, 0))
                             * Matrix4::Scale(Vector3(1, 1, 1));

    sun->localTransform = Matrix4::Translation(Vector3(0, 0, 0)) * Matrix4::Scale(Vector3(3, 3, 3));

    sun->mesh->SetColor(1.0, 170.0f / 255.0f, 0.0, 1.0);
    planet->mesh->SetColor(0, 0, 190.0f / 255.0f, 1.0);
    planet2->mesh->SetColor(120.0f/255.0f, 0, 0, 1.0);
    planet3->mesh->SetColor(0, 230.0f/255.0f, 0, 1.0);

    sun->shader = new Shader(SHADERPATH "SunVert.glsl", SHADERPATH "SunFrag.glsl");
    planet->shader = new Shader(SHADERPATH "PlanetVert.glsl", SHADERPATH "PlanetFrag.glsl");
    planet2->shader = planet->shader;
    planet3->shader = planet->shader;

    light = new Light(Vector3(0, 0, -10), Vector4(1,1,1,1), 50);
    planet->light = light;
    planet2->light = light;
    planet3->light = light;

    if (!sun->shader->LoadSuccess() || !planet->shader->LoadSuccess()) {
        std::cout << "Fuck" << std::endl;
        return;
    }

    cubemap = new Cubemap();

    projMatrix = Matrix4::Perspective(0.01f, 15000.0f, (float)width/float(height), 45.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    init = true;
}

Renderer::~Renderer() {
    delete sun;
    delete planet2;
    delete planet3;
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
    planet2->Draw(this);
    planet3->Draw(this);

}

void Renderer::UpdateScene(float dt) {
    camera->UpdateCamera(dt);
    viewMatrix = camera->BuildViewMatrix();
}

void Model::Draw(Renderer *context) {
    context->BindShader(shader);
    context->UpdateShaderMatrices();

    if (light) {
        glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)&context->camera->Position);
        context->SetShaderLight(*light);
    }

    glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, localTransform.values);
    mesh->Draw();
}
