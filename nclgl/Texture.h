//
// Created by jdhyd on 10/28/2023.
//

#ifndef CSC8502_2022_TEXTURE_H
#define CSC8502_2022_TEXTURE_H

#include <iostream>
#include "stb/stb_image.h"
#include "glad/glad.h"

#define TEXTUREPATH "../textures/"

class Texture {
public:
    static GLuint LoadTexture(const char *filename);
};


#endif //CSC8502_2022_TEXTURE_H
