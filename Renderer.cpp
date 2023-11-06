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

    finalQuad = Mesh::GenerateQuad();

    sun->mesh = Mesh::GenerateUVSphere(30, 20);
    planet->mesh = Mesh::GenerateUVSphere(21, 21);
    planet2->mesh = Mesh::GenerateUVSphere(21, 18);
    planet3->mesh = Mesh::GenerateUVSphere(21, 21);

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

    sun->mesh->SetColor(5, 3.4, 0.0, 1.0);
    planet->mesh->SetColor(0, 0, 190.0f / 255.0f, 1.0);
    planet2->mesh->SetColor(120.0f/255.0f, 0, 0, 1.0);
    planet3->mesh->SetColor(0, 230.0f/255.0f, 0, 1.0);

    sun->shader = new Shader(SHADERPATH "SunVert.glsl", SHADERPATH "SunFrag.glsl");
    planet->shader = new Shader(SHADERPATH "PlanetVert.glsl", SHADERPATH "PlanetFrag.glsl");
    planet2->shader = planet->shader;
    planet3->shader = planet->shader;

    light = new Light(Vector3(0, 0, 0), Vector4(1,1,1,1), 50);
    planet->light = light;
    planet2->light = light;
    planet3->light = light;

    if (!sun->shader->LoadSuccess() || !planet->shader->LoadSuccess()) {
        std::cout << "Fuck" << std::endl;
        return;
    }

    cubemap = new Cubemap();

    hdrShader = new Shader(SHADERPATH "QuadVert.glsl",SHADERPATH "HdrFrag.glsl");

    glGenFramebuffers(1, &hdrFramebuffer);

    // Large fp color texture
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is joever" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bloomRenderer = new BloomRenderer(width, height);

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
    RenderSceneToBuffer();
    bloomRenderer->RenderBloomTexture(colorBuffer, 0.005f, *this);
    RenderTextureToScreen(bloomRenderer->BloomTexture());
}

void Renderer::UpdateScene(float dt) {
    camera->UpdateCamera(dt);
    viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderSceneToBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFramebuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is joever" << std::endl;
    }
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    cubemap->Draw(this);
    sun->Draw(this);
    planet->Draw(this);
    planet2->Draw(this);
    planet3->Draw(this);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderTextureToScreen(GLuint texture) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    BindShader(hdrShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1f(glGetUniformLocation(hdrShader->GetProgram(), "exposure"), 0.5f);
    finalQuad->Draw();
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

bool bloomFBO::Init(GLuint windowWidth, GLuint windowHeight, GLuint mipNumber) {
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
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

        mMipChain.emplace_back(mip);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, mMipChain[0].texture, 0);

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
    mInit = true;
    return true;
}

void bloomFBO::Destroy() {
    for (int i = 0; i < mMipChain.size(); i++) {
        glDeleteTextures(1, &mMipChain[i].texture);
        mMipChain[i].texture = 0;
    }
    glDeleteFramebuffers(1, &mFBO);
    mFBO = 0;
}

void bloomFBO::BindForWriting() {
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is joever" << std::endl;
    }
}

const std::vector<bloomMip> &bloomFBO::MipChain() const {
    return mMipChain;
}

BloomRenderer::BloomRenderer(int windowWidth, int windowHeight) {
    mSrcViewportSize = iVector2{windowWidth, windowHeight};
    mSrcViewportSizeFloat = Vector2((float)windowWidth, (float)windowHeight);

    bool status = mFBO.Init(windowWidth, windowHeight, 8);
    if (!status) {
        std::cerr << "Failed to initialize bloom FBO - cannot create bloom renderer!\n";
        return;
    }

    quad = Mesh::GenerateQuad();

    // Shaders
    mDownsampleShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "Downsample.glsl");
    mUpsampleShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "Upsample.glsl");

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
    mFBO.Destroy();
    delete mDownsampleShader;
    delete mUpsampleShader;
}

void BloomRenderer::RenderBloomTexture(unsigned int srcTexture, float filterRadius, Renderer& context) {
    mFBO.BindForWriting();

    RenderDownsamples(srcTexture, context);
    RenderUpsamples(filterRadius, context);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mSrcViewportSize.x, mSrcViewportSize.y);
}

unsigned int BloomRenderer::BloomTexture() {
    return mFBO.MipChain()[0].texture;
}

void BloomRenderer::RenderDownsamples(unsigned int srcTexture, Renderer& context) {
    const std::vector<bloomMip>& mipChain = mFBO.MipChain();

    context.BindShader(mDownsampleShader);
    glUniform2f(glGetUniformLocation(mDownsampleShader->GetProgram(), "srcResolution"), mSrcViewportSizeFloat.x, mSrcViewportSizeFloat.y);

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
        glUniform2f(glGetUniformLocation(mDownsampleShader->GetProgram(), "srcResolution"), mip.size.x, mip.size.y);
        // Set current mip as texture input for next iteration
        glBindTexture(GL_TEXTURE_2D, mip.texture);
    }
}

void BloomRenderer::RenderUpsamples(float filterRadius, Renderer& context) {
    const std::vector<bloomMip>& mipChain = mFBO.MipChain();

    context.BindShader(mUpsampleShader);
    glUniform1f(glGetUniformLocation(mUpsampleShader->GetProgram(), "filterRadius"), filterRadius);

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
