#pragma once
#include <GL/glew.h>
#include <string>
#include <FreeImage.h>

class TextureLoader {
public:
    static void init();
    static void shutdown();
    static GLuint loadTexture(const std::string& path);

private:
    TextureLoader() = delete; 
    ~TextureLoader() = delete;
};