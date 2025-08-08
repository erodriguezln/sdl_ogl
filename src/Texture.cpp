#include "../include/Texture.h"
#include <glad/glad.h>
#include <iostream>

Texture::Texture(const std::string &path) : path(path), width(0), height(0) {
    // genera el id para la textura &id passed as reference so it can be written
    glGenTextures(1, &id);
    loadData();
}

Texture::~Texture() {
    if (id) {
        // TODO CHECK THIS
        // glDeleteTextures(1, &id);
    }
}

void Texture::loadData() {
    surface = IMG_Load(path.c_str());
    if (!surface) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), SDL_GetError());
        return;
    }

    SDL_FlipSurface(surface, SDL_FLIP_VERTICAL);

    width = surface->w;
    height = surface->h;
    pixelDetails = SDL_GetPixelFormatDetails(surface->format);

    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);


    unsigned int format = GL_RGB;
    if (pixelDetails->bytes_per_pixel == 4) {
        format = GL_RGBA;
    } else if (pixelDetails->bytes_per_pixel == 3) {
        format = GL_RGB;
    } else {
        printf("Unsupported image format: %d bytes per pixel\n",
               pixelDetails->bytes_per_pixel);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_DestroySurface(surface);
}

unsigned int Texture::getID() const {
    return id;
}