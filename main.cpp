#include <iostream>



#include "Window.h"
#include "Renderer.h"
#include "Texture.h"

int main() {
    Window w("Planet Landing!", 1280, 720, false);
    if (!w.HasInitialised()) {
        return -1;
    }

    Renderer renderer(w);
    if (!renderer.HasInitialised()) {
        return -1;
    }

    w.LockMouseToWindow(true);
    w.ShowOSPointer(false);

    while(w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
        renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
        renderer.RenderScene();
        renderer.SwapBuffers();

        if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
            Shader::ReloadAllShaders();
        }
    }
    return 0;
}
