//
// Created by jdhyd on 11/20/2023.
//

#ifndef PLANETLANDING_BLOOMRENDERER_H
#define PLANETLANDING_BLOOMRENDERER_H

#include "Vector2.h"
#include "glad/glad.h"
#include <vector>
#include "Shader.h"

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
    bloomFBO() : init(false) {};
    ~bloomFBO() = default;
    bool Init(GLuint windowWidth, GLuint windowHeight, GLuint mipNumber);
    void Destroy();
    void BindForWriting();
    const std::vector<bloomMip>& MipChain() const;
private:
    unsigned int fbo;
    std::vector<bloomMip> mipChain;
    bool init;
};

class BloomRenderer {
public:
    BloomRenderer(int windowWidth, int windowHeight);
    ~BloomRenderer() = default;
    void Destroy();
    void RenderBloomTexture(unsigned int srcTexture, float filterRadius, Renderer& context);
    unsigned int BloomTexture();
    unsigned int FinalTexture();
private:
    void RenderDownsamples(unsigned int srcTexture, Renderer& context);
    void RenderUpsamples(float filterRadius, Renderer& context);
    void Prefilter(unsigned int srcTexture, Renderer &context);
    void PostFilter(unsigned int srcTexture, unsigned int bloomTexture, Renderer &context);


    bloomFBO fbo;
    iVector2 srcViewport;
    Vector2 srcViewportf;
    Shader* downsampleShader;
    Shader* upsampleShader;
    Shader* prefilterShader;
    Shader* postfilterShader;
    GLuint prefilterTexture;
    GLuint postfilterTexture;
    Mesh* quad;
};



#endif //PLANETLANDING_BLOOMRENDERER_H
