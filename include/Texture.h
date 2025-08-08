#ifndef SDL_OGL_TEXTURE_H
#define SDL_OGL_TEXTURE_H
#include <string>
#include <SDL3_image/SDL_image.h>


class Texture {
public:
    Texture(const std::string &path);

    ~Texture();

    void loadData();

    unsigned int getID() const;

private:
    std::string path;
    SDL_Surface *surface;
    SDL_PixelFormat pixelFormat;

    unsigned int width;
    unsigned int height;
    unsigned int id;
    const SDL_PixelFormatDetails *pixelDetails;
    unsigned int bytesPerPixel;
};


#endif //SDL_OGL_TEXTURE_H