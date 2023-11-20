//
// Created by jdhyd on 10/30/2023.
//

#include "Renderer.h"

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

void Renderer::SnapToStart() {
    camera->Position = Vector3(11, 2, -7);
    camera->Yaw = 140.208;
    camera->Pitch = -16.4501;
}

Renderer::Renderer(Window &parent) : OGLRenderer(parent) {

    shipTransform = Matrix4::Translation(Vector3(1.35897,-0.2,0.497561)) * Matrix4::Rotation(45, Vector3(0, 1, 0)) * Matrix4::Scale(Vector3(0.5, 0.5, 0.5));
    onePlanetTexture = Texture::LoadTexture(TEXTUREPATH "insaneTex.png");

    RegisterComponents();
    LoadShaders();
    CreateTextures();

    BuildScene();
    CreateCameraQueue();

    SnapToStart();
    finalQuad = Mesh::GenerateQuad();

    hdrFramebuffer = CreateHdrFramebuffer(colorBuffer, depthTexture);
    bloomRenderer = new BloomRenderer(width, height);
    InitAntiAliasing();

    projMatrix = Matrix4::Perspective(0.01f, 100.0f, (float)width/float(height), 45.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    init = true;
}

Renderer::~Renderer() {
    //delete sun;
    delete camera;
    delete light;
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
        -7.81998
    });

    cameraQueue.push_back({
          Vector3(12.224,0.500375,-0.900616),
          98.7102,
          -8.09998
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
          -36.3099
    });

    cameraQueue.push_back({
        Vector3(-14.2685,14.4593,3.25974),
        276.611,
        -32.9698
    });

    cameraQueue.push_back({
        Vector3(-11.4415,9.48623,12.0403),
        305.102,
        -23.5898

    });

    cameraQueue.push_back({
        Vector3(2.37111,2.70608,14.3758),
        320.95325,
        -10.9199
    });

    spaceshipTrack = cameraQueue;

    currentTrack = 0;
    currentElapsed = 0;
    trackDuration = 1.0f;
    currentSegment = CalculateSegment(cameraQueue[0].point, cameraQueue[1].point, cameraQueue[2].point, cameraQueue[3].point);
}


Entity Renderer::RegisterPlanet(const Transform& transform, const Vector4& color, Shader* shader, GLuint texture) {
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
    return e;
}

Entity Renderer::RegisterMesh(Registry& r, const Transform& t, Shader* shader, Mesh* mesh) {
    auto e = r.CreateEntity();
    r.AddComponent<Transform>(e, t);
    auto pData = Mesh::ToModelData(mesh);
    pData.shader = shader;
    r.AddComponent<ModelData>(e, pData);
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

    landRegistry.RegisterComponent<Transform>();
    landRegistry.RegisterComponent<ModelData>();
}

void Renderer::LoadShaders() {
    planetShader = new Shader(SHADERPATH "PlanetVert.glsl", SHADERPATH "PlanetFrag.glsl");
    sunShader = new Shader(SHADERPATH "SunVert.glsl", SHADERPATH "SunFrag.glsl");
    hdrShader = new Shader(SHADERPATH "QuadVert.glsl",SHADERPATH "HdrFrag.glsl");
    antiShader = new Shader(SHADERPATH "QuadVert.glsl", SHADERPATH "AntiAliasing.glsl");
    shipShader = new Shader(SHADERPATH "BaseVertex.glsl", SHADERPATH "BaseFragment.glsl");
    reflectiveShader = new Shader(SHADERPATH "reflectiveVertex.glsl", SHADERPATH "reflectiveFragment.glsl");


    if (!sunShader->LoadSuccess()
        || !planetShader->LoadSuccess()
        || !hdrShader->LoadSuccess()
        || !reflectiveShader->LoadSuccess()
        || !shipShader->LoadSuccess()
        || !antiShader->LoadSuccess()) {
        std::cout << "Fuck" << std::endl;
        return;
    }
}

void Renderer::CreateTextures() {
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
                   planetShader, onePlanetTexture);

    RegisterPlanet(Matrix4::Translation(Vector3(15, 0, -10))
                   * Matrix4::Scale(Vector3(0.5, 0.5, 0.5)),
                   Vector4(120.0f/255.0f, 0, 0, 1.0),
                   planetShader, onePlanetTexture);

    RegisterPlanet(
                   Matrix4::Translation(Vector3(-20, 0, 10))
                   * Matrix4::Scale(Vector3(0.5, 0.5, 0.5)),
                   Vector4(0, 230.0f/255.0f, 0, 1.0),
                   planetShader, onePlanetTexture);

    RegisterPlanet(Matrix4::Scale(Vector3(3, 3, 3)), Vector4(2.5, 1.7, 0.5, 1.0), sunShader, 0);

    shipModel = Mesh::LoadFromObjFile(MODELPATH "craft_speederA.obj");
    treeModel = Mesh::LoadFromObjFile(MODELPATH "tree.obj");
    rockMesh = Mesh::LoadFromObjFile(MODELPATH "rockFlatGrass.obj");

    waterQuad = Mesh::GenerateQuad();
    auto waterQuadTransform = Matrix4::Translation(Vector3(0, -1, 0))
                              * Matrix4::Rotation(-90, Vector3(1, 0, 0))
                              * Matrix4::Scale(Vector3(20, 20, 20));
    waterQuad->SetColor(0, 0, 1, 1.0);



    RegisterMesh(landRegistry, shipTransform, shipShader, shipModel);
    auto rockTransform = Matrix4::Translation(Vector3(0, -1, 0)) * Matrix4::Scale(Vector3(5, 5, 5));
    RegisterMesh(landRegistry, rockTransform, shipShader, rockMesh);

    RegisterMesh(landRegistry,  Matrix4::Translation(Vector3(-1.72261,-0.2,1.68769)), shipShader, treeModel);
    RegisterMesh(landRegistry,  Matrix4::Translation(Vector3(2.78209, -0.2, -0.547373)), shipShader, treeModel);
    RegisterMesh(landRegistry,  Matrix4::Translation(Vector3(1.93705, -0.6, -2.769)), shipShader, treeModel);
    RegisterMesh(landRegistry,  Matrix4::Translation(Vector3(-1.57919,-0.2,-0.262525)), shipShader, treeModel);
    RegisterMesh(landRegistry, waterQuadTransform, reflectiveShader, waterQuad);
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

    if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_O) && inSpace) {
        SetAutoCamera();
    }

    if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Q)) {
        std::cout << camera->Position;
        std::cout << camera->Pitch << std::endl;
        std::cout << camera->Yaw << std::endl;
    }

    if (autoPilot)
        MovePosition(dt);
    else {
        UpdateLookDirection(dt);
        TranslateCamera(dt);
    }


    //UpdateShip(dt);
    viewMatrix = camera->BuildViewMatrix();
}

void Renderer::UpdateScene(float dt) {
    UpdateCameraMovement(dt);
    if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_X)) {
        if (inSpace) {

            for (auto &t : registry.GetComponents<Transform>()->items) {
                if (TestSphereIntersect(t.GetPositionVector(), 0.5)) {
                    inSpace = false;
                    camera->Position = Vector3(0, 0, 0);
                    light->Position = Vector3(0, 10, 10);
                    light->Color = Vector4(1.5, 1.5, 1.5, 1.0);
                    break;
                }
            }

        } else {
            inSpace = true;
            light->Position = Vector3(0, 0, 0);
            light->Color = Vector4(0.6, 0.6, 0.8, 1.0);
            SnapToStart();
        }
    }
}

void Renderer::SetAutoCamera() {
    autoPilot = !autoPilot;
}

void Renderer::RenderSceneToBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFramebuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is joever" << std::endl;
    }
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    cubemap->Draw(this);

    //DrawShip();
    DrawModels();
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

void Renderer::DrawModels() {

    auto &r = inSpace ? registry : landRegistry;


    auto c = r.GetComponents<ModelData>();
    auto t = r.GetComponents<Transform>();
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
            glUniform1i(glGetUniformLocation(item.shader->GetProgram(), "glowFactor"), shouldIntersect);

            Vector3 v2 =  (Matrix4::Rotation(camera->Yaw, Vector3(0, 1, 0))
                           * Matrix4::Rotation(camera->Pitch, Vector3(1, 0, 0))
                           * Matrix4::Translation(Vector3(0, 0, -1)))

                                  .GetPositionVector();

            glUniform3fv(glGetUniformLocation(item.shader->GetProgram(), "viewVector"), 1, (float*)&v2);
        }

        if (item.shader == reflectiveShader) {
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->textureId);
        }

        glUniformMatrix4fv(glGetUniformLocation(item.shader->GetProgram(), "modelMatrix"), 1, false, t->items[i].values);

        glBindVertexArray(item.arrayObject);
        glDrawElements(item.type, item.numIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

int Renderer::TestSphereIntersect(const Vector3 &position, float scale) {
    Vector3 v1 = camera->Position;
    Vector3 v2 =  (Matrix4::Rotation(camera->Yaw, Vector3(0, 1, 0))
                 * Matrix4::Rotation(camera->Pitch, Vector3(1, 0, 0))
                 * Matrix4::Translation(Vector3(0, 0, -1)))

                 .GetPositionVector() * 30 + camera->Position;

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

