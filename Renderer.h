//
// Created by jdhyd on 10/30/2023.
//

#ifndef PLANETLANDING_RENDERER_H
#define PLANETLANDING_RENDERER_H

#include "OGLRenderer.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "Cubemap.h"
#include "Light.h"
#include <memory>

#define SHADERPATH "../shaders/"

class Renderer;

class Model {
public:
    Model() = default;
    ~Model() = default;
    Mesh* mesh;
    GLuint* texture;

    Matrix4 localTransform;
    Shader* shader;
    Light* light;

    void Draw(Renderer *context);

};

class Renderer: public OGLRenderer {
public:
    Renderer(Window& parent);
    ~Renderer();

    void RenderScene() override;
    void UpdateScene(float dt) override;
public:
    Model *sun, *planet;
    Camera* camera;
    Light* light;
    Cubemap* cubemap;
};


#endif //PLANETLANDING_RENDERER_H
