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

#define SHADERPATH "../shaders/"


class Renderer: public OGLRenderer {
public:
    Renderer(Window& parent);
    ~Renderer();

    void RenderScene() override;
    void UpdateScene(float dt) override;
protected:
    Mesh* sphere;
    Mesh* quad;
    Camera* camera;
    Shader* shader;
    GLuint texture;
    Cubemap* cubemap;
};


#endif //PLANETLANDING_RENDERER_H
