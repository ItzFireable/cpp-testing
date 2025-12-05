#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"

class SpriteBatch {
public:
    SpriteBatch();
    ~SpriteBatch();

    void init(float screenWidth, float screenHeight);
    void begin();
    void drawTexture(GLuint texture, float x, float y, float w, float h, 
                     const glm::vec4& color = glm::vec4(1.0f));
    void drawRect(float x, float y, float w, float h, const glm::vec4& color);
    void end();
    void setProjection(float width, float height);

private:
    GLuint vao, vbo, ebo;
    Shader* textureShader;
    Shader* colorShader;
    glm::mat4 projection;
};