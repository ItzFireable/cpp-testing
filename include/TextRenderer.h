#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>
#include "Shader.h"

struct Character {
    GLuint textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    bool init(const std::string& fontPath, unsigned int fontSize);
    void renderText(const std::string& text, float x, float y, float scale, 
                   const glm::vec4& color, float screenWidth, float screenHeight);
    
    glm::vec2 measureText(const std::string& text, float scale);
    float getLineHeight(float scale);

private:
    std::map<char, Character> characters_;
    FT_Library ft_;
    FT_Face face_;
    Shader* shader_;
    GLuint vao_, vbo_;
    
    bool loadFont(const std::string& fontPath, unsigned int fontSize);
};