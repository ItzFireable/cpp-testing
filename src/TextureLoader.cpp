#include "TextureLoader.h"
#include <iostream>

void TextureLoader::init() {
    FreeImage_Initialise(TRUE);
    std::cout << "FreeImage initialized." << std::endl;
}

void TextureLoader::shutdown() {
    FreeImage_DeInitialise();
    std::cout << "FreeImage shut down." << std::endl;
}

GLuint TextureLoader::loadTexture(const std::string& path) {
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path.c_str(), 0);
    if (format == FIF_UNKNOWN) {
        format = FreeImage_GetFIFFromFilename(path.c_str());
        if (format == FIF_UNKNOWN) {
            std::cerr << "Error: Unknown image format for " << path << std::endl;
            return 0;
        }
    }

    FIBITMAP* dib = FreeImage_Load(format, path.c_str());
    if (!dib) {
        std::cerr << "Error: Failed to load image " << path << std::endl;
        return 0;
    }

    FIBITMAP* temp = dib;
    dib = FreeImage_ConvertTo32Bits(temp);
    FreeImage_Unload(temp);

    if (!dib) {
        std::cerr << "Error: Failed to convert image to 32-bit for " << path << std::endl;
        return 0;
    }

    int width  = FreeImage_GetWidth(dib);
    int height = FreeImage_GetHeight(dib);
    void* bits = FreeImage_GetBits(dib);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                 GL_BGRA, GL_UNSIGNED_BYTE, bits); 
    
    glGenerateMipmap(GL_TEXTURE_2D);
    FreeImage_Unload(dib);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}