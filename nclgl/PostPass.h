//
// Created by jdhyd on 11/4/2023.
//

#ifndef PLANETLANDING_POSTPASS_H
#define PLANETLANDING_POSTPASS_H

#include <glad/glad.h>
#include <iostream>

class PostPass {
    PostPass(int width, int height, int oWidth, int oHeight);
    ~PostPass();
private:
    GLuint index;
    GLuint textureColorBuffer;
};


#endif //PLANETLANDING_POSTPASS_H
