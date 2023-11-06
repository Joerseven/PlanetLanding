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

struct iVector2 {
    int x;
    int y;
};

struct bloomMip {
    Vector2 size;
    iVector2 iSize;
    GLuint texture;
};

class bloomFBO {
public:
    bloomFBO() : mInit(false) {};
    ~bloomFBO() = default;
    bool Init(GLuint windowWidth, GLuint windowHeight, GLuint mipNumber);
    void Destroy();
    void BindForWriting();
    const std::vector<bloomMip>& MipChain() const;
private:
    unsigned int mFBO;
    std::vector<bloomMip> mMipChain;
    bool mInit;
};

class BloomRenderer {
public:
    BloomRenderer(int windowWidth, int windowHeight);
    ~BloomRenderer() = default;
    void Destroy();
    void RenderBloomTexture(unsigned int srcTexture, float filterRadius, Renderer& context);
    unsigned int BloomTexture();
private:
    void RenderDownsamples(unsigned int srcTexture, Renderer& context);
    void RenderUpsamples(float filterRadius, Renderer& context);

    bloomFBO mFBO;
    iVector2 mSrcViewportSize;
    Vector2 mSrcViewportSizeFloat;
    Shader* mDownsampleShader;
    Shader* mUpsampleShader;
    Mesh* quad;
};



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
    Model *sun, *planet, *planet2, *planet3;
    Camera* camera;
    Mesh* finalQuad;
    Light* light;
    Cubemap* cubemap;
    Shader* hdrShader;
    GLuint colorBuffer;
    GLuint hdrFramebuffer;
    GLuint depthRenderbuffer;
    BloomRenderer* bloomRenderer;

    void RenderSceneToBuffer();

    void RenderTextureToScreen(GLuint texture);
};


#endif //PLANETLANDING_RENDERER_H
