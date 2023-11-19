//
// Created by jdhyd on 10/30/2023.
//

#include "Renderer.h"

//    AddCameraAnimation({
//        Vector3(5.790, 0.215, 40.784),
//        Vector3(12.0974,0.327629,16.9606),
//        Vector3(15.3172,3.30724,12.3732),
//        Vector3(30.3667,8.63543,10.8772),
//        20.0f,
//        0.0f
//    });

float lerp(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}

Segment CalculateSegment(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3) {
    float alpha = 0.5f;
    float tension = 0;
    float t0 = 0.0f;

    float t1 = t0 + pow((p1 - p0).Length(), alpha);
    float t2 = t1 + pow((p2 - p1).Length(), alpha);
    float t3 = t2 + pow((p3 - p2).Length(), alpha);

    Vector3 m1 = ((p1 - p0) / (t1 - t0) - (p2 - p0) / (t2 - t0) + (p2 - p1) / (t2 - t1)) * (1.0f - tension) * (t2 - t1);
    Vector3 m2 = ((p2 - p1) / (t2 - t1) - (p3 - p1) / (t3 - t1) + (p3 - p2) / (t3 - t2)) * (1.0f - tension) * (t2 - t1);

    Segment segment;
    segment.a = (p1 - p2) * 2.0f + m1 + m2;
    segment.b = (p1 - p2) * -3.0f - m1 - m1 - m2;
    segment.c = m1;
    segment.d = p1;
    return segment;
}

Vector3 GetPoint(const Segment& segment, float t) {
    return segment.a * t * t * t
        + segment.b * t * t
        + segment.c * t
        + segment.d;
}

// Use bsplines as they're c2 continuous and will be smooooth
Vector3 BSplinePoint(Vector3 &ControlPoint1, Vector3 &ControlPoint2, Vector3 &ControlPoint3, Vector3 &ControlPoint4, float t) {

    Vector3 DrawCurve = {0, 0, 0};

    float t2 = t * t;
    float t3 = t2 * t;
    float mt3 = (1 - t) * (1 - t) * (1 - t);

    float bi3 = mt3 / 6;
    float bi2 = ((3 * t3) - (6 * t2) + 4) / 6;
    float bi1 = ((-3 * t3) + (3 * t2) + (3 * t) + 1) / 6;
    float bi  = mt3 / 6;

    DrawCurve.x = ControlPoint1.x * bi3;
    DrawCurve.x += ControlPoint2.x * bi2;
    DrawCurve.x += ControlPoint3.x * bi1;
    DrawCurve.x += ControlPoint4.x * bi;

    DrawCurve.y = ControlPoint1.y * bi3;
    DrawCurve.y += ControlPoint2.y * bi2;
    DrawCurve.y += ControlPoint3.y * bi1;
    DrawCurve.y += ControlPoint4.y * bi;

    DrawCurve.z = ControlPoint1.z * bi3;
    DrawCurve.z += ControlPoint2.z * bi2;
    DrawCurve.z += ControlPoint3.z * bi1;
    DrawCurve.z += ControlPoint4.z * bi;

    return DrawCurve;
}

Renderer::Renderer(Window &parent) : OGLRenderer(parent) {

    RegisterComponents();
    LoadShaders();
    CreateTextures();

    BuildScene();
    CreateCameraQueue();

    finalQuad = Mesh::GenerateQuad();

    hdrFramebuffer = CreateHdrFramebuffer(colorBuffer, depthTexture);
    bloomRenderer = new BloomRenderer(width, height);
    InitAntiAliasing();

    projMatrix = Matrix4::Perspective(0.01f, 100.0f, (float)width/float(height), 45.0f);
    shipTransform = Matrix4::Translation(Vector3(10, 10, 10)) * Matrix4::Rotation(0, Vector3(0, 1, 0)) * Matrix4::Scale(Vector3(1, 1, 1));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    init = true;
}

Renderer::~Renderer() {
    //delete sun;
    delete camera;
    delete light;
    delete noise;
}

void Renderer::CreateCameraQueue() {

    cameraQueue.push_back({
      Vector3(10.8239,0.100012,0.0983553),
      86.6702,
      -8.65998,
    });

    cameraQueue.push_back({
        Vector3(11.2352,0.100012,0.00594361),
        84.4302,
        -7.81998,
    });

    cameraQueue.push_back({
          Vector3(12.224,0.500375,-0.900616),
          98.7102,
          -8.09998,
    });

    cameraQueue.push_back({
          Vector3(14.4813,2.8122,-5.92533),
          122.86,
          -15.45
    });

    cameraQueue.push_back({
          Vector3(14.1931,5.99043,-13.2191),
          143.581,
          -19.7899
    });

    cameraQueue.push_back({
          Vector3(4.92461,12.2665,-13.0478),
          162.691,
          -36.3099,
    });

    spaceshipTrack = cameraQueue;

    currentTrack = 0;
    currentElapsed = 0;
    trackDuration = 1.0f;
    currentSegment = CalculateSegment(cameraQueue[0].point, cameraQueue[1].point, cameraQueue[2].point, cameraQueue[3].point);
}


Entity Renderer::RegisterPlanet(Transform transform, Vector4 color, Shader* shader, GLuint texture) {
    auto e = registry.CreateEntity();
    registry.AddComponent<Transform>(e, transform);
    auto pMesh = Mesh::GenerateUVSphere(30, 30);
    pMesh->SetColor(color.x, color.y, color.z, color.w);
    auto pData = Mesh::ToModelData(pMesh);
    pData.shader = shader;
    if (texture != 0 ) {
        pData.texture = texture;
    }
    registry.AddComponent<ModelData>(e, pData);
    // I know it doesn't delete mesh :(
    return e;
}

GLuint Renderer::CreatePostPassTexture() {
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return t;
}

GLuint Renderer::CreateDepthTexture() {
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return t;
}

GLuint Renderer::CreateHdrFramebuffer(GLuint colorBuffer, GLuint depthTexture) {
    GLuint f;
    glGenFramebuffers(1, &f);
    glBindFramebuffer(GL_FRAMEBUFFER, f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

    if (depthTexture != 0) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is jover" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return f;
}

void Renderer::RegisterComponents() {
    registry.RegisterComponent<Transform>();
    registry.RegisterComponent<ModelData>();
}

void Renderer::LoadShaders() {
    planetShader = new Shader(SHADERPATH "PlanetVert.glsl", SHADERPATH "PlanetFrag.glsl");
    sunShader = new Shader(SHADERPATH "SunVert.glsl", SHADERPATH "SunFrag.glsl");
    hdrShader = new Shader(SHADERPATH "QuadVert.glsl",SHADERPATH "HdrFrag.glsl");
    antiShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "AntiAliasing.glsl");
    shipShader = new Shader(SHADERPATH "BaseVertex.glsl", SHADERPATH "BaseFragment.glsl");


    if (!sunShader->LoadSuccess()
        || !planetShader->LoadSuccess()
        || !hdrShader->LoadSuccess()
        || !shipShader->LoadSuccess()
        || !antiShader->LoadSuccess()) {
        std::cout << "Fuck" << std::endl;
        return;
    }
}

void Renderer::CreateTextures() {
    noise = new Noise(4096, 4096);
    colorBuffer = CreatePostPassTexture();
    depthTexture = CreateDepthTexture();
}

void Renderer::BuildScene() {

    camera = new Camera();

    light = new Light(Vector3(0, 0, 0), Vector4(1.5,1.5,1.5,1.5), 50);

    cubemap = new Cubemap();

    RegisterPlanet(Matrix4::Translation(Vector3(10, 0, 0))
                   * Matrix4::Scale(Vector3(0.5, 0.5, 0.5)),
                   Vector4(0, 0, 190.0f / 255.0f, 1.0),
                   planetShader, noise->texture);

    RegisterPlanet(Matrix4::Translation(Vector3(20, 0, 0))
                   * Matrix4::Scale(Vector3(0.5, 0.5, 0.5)),
                   Vector4(120.0f/255.0f, 0, 0, 1.0),
                   planetShader, noise->texture);

    RegisterPlanet(
                   Matrix4::Translation(Vector3(30, 0, 0))
                   * Matrix4::Scale(Vector3(0.5, 0.5, 0.5)),
                   Vector4(0, 230.0f/255.0f, 0, 1.0),
                   planetShader, noise->texture);

    RegisterPlanet(Matrix4::Scale(Vector3(3, 3, 3)), Vector4(2.5, 1.7, 0.5, 1.0), sunShader, 0);

    shipModel = Mesh::LoadFromObjFile(MODELPATH "craft_speederA.obj");
}

void Renderer::InitAntiAliasing() {
    antiATex = CreatePostPassTexture();
    antiABuffer = CreateHdrFramebuffer(antiATex, 0);
}

void Renderer::AntiAliasingPass(GLuint tex) {
    glBindFramebuffer(GL_FRAMEBUFFER, antiABuffer);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    BindShader(antiShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1f(glGetUniformLocation(antiShader->GetProgram(), "width"), width);
    glUniform1f(glGetUniformLocation(antiShader->GetProgram(), "height"), height);
    finalQuad->Draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderScene() {
    RenderSceneToBuffer();

    bloomRenderer->RenderBloomTexture(colorBuffer, 0.01f, *this);
    AntiAliasingPass(bloomRenderer->FinalTexture());
    RenderTextureToScreen(antiATex);
}

void Renderer::TranslateCamera(float dt) {
    Vector3 direction = {0, 0, 0};

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
        direction += {0, 0, -1};
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
        direction += {0, 0, 1};
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
        direction += {-1, 0, 0};
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
        direction += {1, 0, 0};
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
        direction += {0, 1, 0};
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
        direction += {0, -1, 0};
    }

    direction.Normalise();

    camera->UpdatePosition(direction, 2.0f * dt);

}

void Renderer::UpdateLookDirection(float dt) const {
   camera->UpdateLookDirection(Window::GetMouse()->GetRelativePosition());
}

bool Renderer::MoveSpaceship(float dt, Vector3& position) {

    int trackSize = (int)cameraQueue.size();
    float t = currentElapsed / trackDuration;

    position = GetPoint(currentSegment, t);


    currentElapsed += dt * 0.2f;
    camera->Pitch = lerp(cameraQueue[(currentTrack+1)%trackSize].pitch, cameraQueue[(currentTrack+2)%trackSize].pitch, t);
    camera->Yaw = lerp(cameraQueue[(currentTrack+1)%trackSize].yaw, cameraQueue[(currentTrack+2)%trackSize].yaw, t);


    if (currentElapsed > trackDuration) {

        currentTrack = (currentTrack + 1) % trackSize;
        currentSegment = CalculateSegment(cameraQueue[currentTrack].point,
                                          cameraQueue[(currentTrack+1)%trackSize].point,
                                          cameraQueue[(currentTrack+2)%trackSize].point,
                                          cameraQueue[(currentTrack+3)%trackSize].point);
        currentElapsed = 0.0f;
    }

    return true;
}

bool Renderer::MovePosition(float dt) {

    int trackSize = (int)cameraQueue.size();
    float t = currentElapsed / trackDuration;

    camera->Position = GetPoint(currentSegment, t);
    std::cout << camera->Pitch << std::endl;
    std::cout << camera->Yaw << std::endl;


    currentElapsed += dt;
    camera->Pitch = lerp(cameraQueue[(currentTrack+1)%trackSize].pitch, cameraQueue[(currentTrack+2)%trackSize].pitch, t);
    camera->Yaw = lerp(cameraQueue[(currentTrack+1)%trackSize].yaw, cameraQueue[(currentTrack+2)%trackSize].yaw, t);


    if (currentElapsed > trackDuration) {

        currentTrack = (currentTrack + 1) % trackSize;
        currentSegment = CalculateSegment(cameraQueue[currentTrack].point,
                                          cameraQueue[(currentTrack+1)%trackSize].point,
                                          cameraQueue[(currentTrack+2)%trackSize].point,
                                          cameraQueue[(currentTrack+3)%trackSize].point);
        currentElapsed = 0.0f;
    }

    return true;
}

void Renderer::UpdateCameraMovement(float dt) {

    SetAutoCamera(false);

    if (autoPilot)
        MovePosition(dt);
    else {
        UpdateLookDirection(dt);
        TranslateCamera(dt);
    }


    //UpdateShip(dt);
    if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Q)) {
        std::cout << camera->Position;
        std::cout << camera->Yaw << std::endl;
        std::cout << camera->Pitch << std::endl;
    }

    viewMatrix = camera->BuildViewMatrix();
}

void Renderer::UpdateScene(float dt) {
    UpdateCameraMovement(dt);
}

void Renderer::SetAutoCamera(bool toggle) {
    autoPilot = toggle;
}

void Renderer::RenderPlanetAtmosphere(GLuint tex, GLuint depth) {
    glBindFramebuffer(GL_FRAMEBUFFER, atmosphereFramebuffer);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    BindShader(atmosphereShader);

    Vector3 planetPos = {10, 0, 0};

    glUniform1i(glGetUniformLocation(atmosphereShader->GetProgram(), "screenTexture"), 0);
    glUniform3fv(glGetUniformLocation(atmosphereShader->GetProgram(), "cameraPosition"), 1, (float*)&camera->Position);
    glUniformMatrix4fv(glGetUniformLocation(atmosphereShader->GetProgram(), "projViewMatrix"), 1, false, (projMatrix * viewMatrix).values);
    glUniformMatrix4fv(glGetUniformLocation(atmosphereShader->GetProgram(), "projMatrix"), 1, false, projMatrix.values);
    glUniformMatrix4fv(glGetUniformLocation(atmosphereShader->GetProgram(), "viewMatrix"), 1, false, viewMatrix.values);
    glUniform3fv(glGetUniformLocation(atmosphereShader->GetProgram(), "planetCenter"), 1, (float*)&planetPos);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glUniform1i(glGetUniformLocation(atmosphereShader->GetProgram(), "depthTexture"), 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depth);

    finalQuad->Draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderSceneToBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFramebuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is joever" << std::endl;
    }
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    cubemap->Draw(this);

    DrawShip();
    DrawModels();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::UpdateShip(float dt) {

    float thrust = 0;
    Vector3 pitchYaw;

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
        thrust += 1;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
        pitchYaw.x -= 1;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
        pitchYaw.x += 1;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
        pitchYaw.y += 1;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
        pitchYaw.y -= 1;
    }
}

void Renderer::DrawShip() {
    BindShader(shipShader);
    modelMatrix = shipTransform;
    UpdateShaderMatrices();
    SetShaderLight(*light);
    glUniform3fv(glGetUniformLocation(shipShader->GetProgram(), "cameraPos"), 1, (float*)&camera->Position);
    shipModel->Draw();
}

void Renderer::RenderTextureToScreen(GLuint texture) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    BindShader(hdrShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1f(glGetUniformLocation(hdrShader->GetProgram(), "exposure"), 0.5f);
    finalQuad->Draw();
}

// Don't really want to use a shader pointer here, it could just be done with the int from get program but it breaks OGL renderer stuff if I don't use 'BindShader'.
void Renderer::DrawModels() {
    auto c = registry.GetComponents<ModelData>();
    auto t = registry.GetComponents<Transform>();
    auto &transforms = t->items;
    auto &modeldata = c->items;
    int i = 0;
    for (auto it = c->items.begin(); it != c->items.end(); it++, i++) {
        auto item = *it;
        BindShader(item.shader);
        UpdateShaderMatrices();

        glUniform3fv(glGetUniformLocation(item.shader->GetProgram(), "cameraPos"), 1, (float*)&camera->Position);
        SetShaderLight(*light);

        if (item.texture != 0) {
            glUniform1i(glGetUniformLocation(item.shader->GetProgram(), "diffuseTex"), 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, item.texture);
        }

        if (item.shader == planetShader) {
            int shouldIntersect = TestSphereIntersect(t->items[i].GetPositionVector(), transforms[i].GetScalingVector().x);
            std::cout << shouldIntersect << std::endl;
        }

        glUniformMatrix4fv(glGetUniformLocation(item.shader->GetProgram(), "modelMatrix"), 1, false, t->items[i].values);

        glBindVertexArray(item.arrayObject);
        glDrawElements(item.type, item.numIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

int Renderer::TestSphereIntersect(const Vector3 &position, float scale) {
    Vector3 v1 = camera->Position;
    Vector3 v2 = (( Matrix4::Rotation(camera->Pitch, Vector3(1, 0, 0))
                 * Matrix4::Rotation(camera->Yaw, Vector3(0, 1, 0))
                 * Matrix4::Translation(Vector3(0, 0, -1))).GetPositionVector() * 30) + camera->Position;


    auto a = v1 - position;
    auto b = v2 - position;
    auto c = v1 - v2;

    if (a.Length() < scale || b.Length() < scale) {
        return 1;
    }

    auto dot = Vector3::Dot(a, b);
    auto mag = a.Length() * b.Length();

    auto theta = acos(dot / mag);

    if (theta < PI / 2)
        return 0;

    auto u1 = a.Normalised();
    auto u2 = c.Normalised();

    auto uDot = Vector3::Dot(u1, u2);

    auto h = a.Length() * sqrt(1 - uDot * uDot);

    if (h < scale)
        return 1;
    if (h >= scale)
        return 0;

    return 0;
}

void Model::Draw(Renderer *context) {
    context->BindShader(shader);
    context->UpdateShaderMatrices();

    if (light) {
        glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)&context->camera->Position);
        context->SetShaderLight(*light);
    }

    if (texture) {
        glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
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
    mPrefilterShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "BloomPrefilter.glsl");
    mPostfilterShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "BloomPostfilter.glsl");

    glGenTextures(1, &mPrefilterTexture);
    glBindTexture(GL_TEXTURE_2D, mPrefilterTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                 windowWidth, windowHeight,
                 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &mPostfilterTexture);
    glBindTexture(GL_TEXTURE_2D, mPostfilterTexture);
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
    mFBO.Destroy();
    delete mDownsampleShader;
    delete mUpsampleShader;
    glDeleteTextures(1, &mPrefilterTexture);
}

void BloomRenderer::RenderBloomTexture(unsigned int srcTexture, float filterRadius, Renderer& context) {
    mFBO.BindForWriting();

    Prefilter(srcTexture, context);
    RenderDownsamples(mPrefilterTexture, context);
    RenderUpsamples(filterRadius, context);
    PostFilter(srcTexture, BloomTexture(), context);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mSrcViewportSize.x, mSrcViewportSize.y);
}

unsigned int BloomRenderer::FinalTexture() {
    return mPostfilterTexture;
}

unsigned int BloomRenderer::BloomTexture() {
    return mFBO.MipChain()[0].texture;
}

void BloomRenderer::Prefilter(unsigned int srcTexture, Renderer& context) {
    context.BindShader(mPrefilterShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPrefilterTexture, 0);
    quad->Draw();
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

void BloomRenderer::PostFilter(unsigned int srcTexture, unsigned int bloomTexture, Renderer &context) {
    context.BindShader(mPostfilterShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    glUniform1i(glGetUniformLocation(mPostfilterShader->GetProgram(), "srcTex"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glUniform1i(glGetUniformLocation(mPostfilterShader->GetProgram(), "bloomTex"), 1);
    glViewport(0, 0, mSrcViewportSize.x, mSrcViewportSize.y);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPostfilterTexture, 0);
    quad->Draw();
}
