//
// Created by jdhyd on 10/28/2023.
//

#include "Texture.h"

GLuint Texture::LoadTexture(const char *filename) {
    std::cout << "Loading texture: " << filename << std::endl;
    int width, height, nrChannels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (!data) {
        std::cout << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    std::cout << "Loaded image with properties: " << std::endl;
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;
    std::cout << "Channels: " << nrChannels << std::endl;

    unsigned int t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, (nrChannels == 3) ? GL_RGB : GL_RGBA, width, height, 0, (nrChannels == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    std::cout << "Bound texture to id: " << t << std::endl;
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}