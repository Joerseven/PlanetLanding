#include "BloomRenderer.h"
#include "Renderer.h"


bool bloomFBO::Init(GLuint windowWidth, GLuint windowHeight, GLuint mipNumber) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is joever" << std::endl;
    }

    Vector2 mipSize((float)windowWidth, (float)windowHeight);
    iVector2 mipIntSize((int)windowWidth, (int)windowHeight);

    for (unsigned int i = 0; i < mipNumber; i++)
    {
        bloomMip mip;

        mipSize = mipSize * 0.5f;
        mipIntSize = iVector2{mipIntSize.x / 2, mipIntSize.y / 2};
        mip.size = mipSize;
        mip.iSize = mipIntSize;

        glGenTextures(1, &mip.texture);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        // we are downscaling an HDR color buffer, so we need a float texture format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                     (int)mipSize.x, (int)mipSize.y,
                     0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        mipChain.emplace_back(mip);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, mipChain[0].texture, 0);

    // setup attachments
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    // check completion status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Bloom Death" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    init = true;
    return true;
}

void bloomFBO::Destroy() {
    for (int i = 0; i < mipChain.size(); i++) {
        glDeleteTextures(1, &mipChain[i].texture);
        mipChain[i].texture = 0;
    }
    glDeleteFramebuffers(1, &fbo);
    fbo = 0;
}

void bloomFBO::BindForWriting() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is joever" << std::endl;
    }
}

const std::vector<bloomMip> &bloomFBO::MipChain() const {
    return mipChain;
}

BloomRenderer::BloomRenderer(int windowWidth, int windowHeight) {
    srcViewport = iVector2{windowWidth, windowHeight};
    srcViewportf = Vector2((float)windowWidth, (float)windowHeight);

    bool status = fbo.Init(windowWidth, windowHeight, 8);
    if (!status) {
        std::cerr << "Failed to initialize bloom FBO - cannot create bloom renderer!\n";
        return;
    }

    quad = Mesh::GenerateQuad();

    // Shaders
    downsampleShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "Downsample.glsl");
    upsampleShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "Upsample.glsl");
    prefilterShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "BloomPrefilter.glsl");
    postfilterShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "BloomPostfilter.glsl");

    glGenTextures(1, &prefilterTexture);
    glBindTexture(GL_TEXTURE_2D, prefilterTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                 windowWidth, windowHeight,
                 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &postfilterTexture);
    glBindTexture(GL_TEXTURE_2D, postfilterTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                 windowWidth, windowHeight,
                 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Don't think I need this iirc
    // Downsample
//    mDownsampleShader->use();
//    mDownsampleShader->setInt("srcTexture", 0);
//    mDownsampleShader->Deactivate();
//
//    // Upsample
//    mUpsampleShader->use();
//    mUpsampleShader->setInt("srcTexture", 0);
//    mUpsampleShader->Deactivate();
}

void BloomRenderer::Destroy() {
    fbo.Destroy();
    delete downsampleShader;
    delete upsampleShader;
    glDeleteTextures(1, &prefilterTexture);
}

void BloomRenderer::RenderBloomTexture(unsigned int srcTexture, float filterRadius, Renderer& context) {
    fbo.BindForWriting();

    Prefilter(srcTexture, context);
    RenderDownsamples(prefilterTexture, context);
    RenderUpsamples(filterRadius, context);
    PostFilter(srcTexture, BloomTexture(), context);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, srcViewport.x, srcViewport.y);
}

unsigned int BloomRenderer::FinalTexture() {
    return postfilterTexture;
}

unsigned int BloomRenderer::BloomTexture() {
    return fbo.MipChain()[0].texture;
}

void BloomRenderer::Prefilter(unsigned int srcTexture, Renderer& context) {
    context.BindShader(prefilterShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prefilterTexture, 0);
    quad->Draw();
}

void BloomRenderer::RenderDownsamples(unsigned int srcTexture, Renderer& context) {
    const std::vector<bloomMip>& mipChain = fbo.MipChain();

    context.BindShader(downsampleShader);
    glUniform2f(glGetUniformLocation(downsampleShader->GetProgram(), "srcResolution"), srcViewportf.x, srcViewportf.y);

    // Bind srcTexture (HDR color buffer) as initial texture input
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);

    // Progressively downsample through the mip chain
    for (int i = 0; i < mipChain.size(); i++)
    {
        const bloomMip& mip = mipChain[i];
        glViewport(0, 0, mip.size.x, mip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, mip.texture, 0);

        // Render screen-filled quad of resolution of current mip
        quad->Draw();

        // Set current mip resolution as srcResolution for next iteration
        glUniform2f(glGetUniformLocation(downsampleShader->GetProgram(), "srcResolution"), mip.size.x, mip.size.y);
        // Set current mip as texture input for next iteration
        glBindTexture(GL_TEXTURE_2D, mip.texture);
    }
}

void BloomRenderer::RenderUpsamples(float filterRadius, Renderer& context) {
    const std::vector<bloomMip>& mipChain = fbo.MipChain();

    context.BindShader(upsampleShader);
    glUniform1f(glGetUniformLocation(upsampleShader->GetProgram(), "filterRadius"), filterRadius);

    // Enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    for (int i = mipChain.size() - 1; i > 0; i--)
    {
        const bloomMip& mip = mipChain[i];
        const bloomMip& nextMip = mipChain[i-1];

        // Bind viewport and texture from where to read
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        // Set framebuffer render target (we write to this texture)
        glViewport(0, 0, nextMip.size.x, nextMip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, nextMip.texture, 0);

        // Render screen-filled quad of resolution of current mip
        quad->Draw();
    }

    // Disable additive blending
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default
    glDisable(GL_BLEND);
}

void BloomRenderer::PostFilter(unsigned int srcTexture, unsigned int bloomTexture, Renderer &context) {
    context.BindShader(postfilterShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    glUniform1i(glGetUniformLocation(postfilterShader->GetProgram(), "srcTex"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glUniform1i(glGetUniformLocation(postfilterShader->GetProgram(), "bloomTex"), 1);
    glViewport(0, 0, srcViewport.x, srcViewport.y);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postfilterTexture, 0);
    quad->Draw();
}