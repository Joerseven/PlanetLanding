//
// Created by jdhyd on 10/30/2023.
//

#ifndef PLANETLANDING_RENDERER_H
#define PLANETLANDING_RENDERER_H

#include "OGLRenderer.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"

#define SHADERPATH "../shaders/"
#define TEXTUREPATH "../textures/"

class Renderer: public OGLRenderer {
public:
    Renderer(Window& parent);
    ~Renderer();

    void RenderScene() override;
    void UpdateScene(float dt) override;
protected:
    Mesh* quad;
    Camera* camera;
    Shader* shader;
    GLuint texture;
};


#endif //PLANETLANDING_RENDERER_H
