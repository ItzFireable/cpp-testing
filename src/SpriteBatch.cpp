#include "Shader.h"
#include "SpriteBatch.h"
#include <glm/gtc/matrix_transform.hpp>

SpriteBatch::SpriteBatch() 
    : vao(0), vbo(0), ebo(0), textureShader(nullptr), colorShader(nullptr) {
}

SpriteBatch::~SpriteBatch() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    delete textureShader;
    delete colorShader;
}

void SpriteBatch::init(float screenWidth, float screenHeight) {
    textureShader = new Shader(Shaders::TEXTURE_VERTEX, Shaders::TEXTURE_FRAGMENT);
    colorShader = new Shader(Shaders::COLOR_VERTEX, Shaders::COLOR_FRAGMENT);

    setProjection(screenWidth, screenHeight);

    float vertices[] = {
        0.0f, 1.0f,     0.0f, 1.0f,
        1.0f, 1.0f,     1.0f, 1.0f,
        1.0f, 0.0f,     1.0f, 0.0f,
        0.0f, 0.0f,     0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void SpriteBatch::begin() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SpriteBatch::drawTexture(GLuint texture, float x, float y, float w, float h, 
                               const glm::vec4& color) {
    textureShader->use();
    textureShader->setMat4("projection", projection);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(w, h, 1.0f));
    
    textureShader->setMat4("model", model);
    textureShader->setVec4("color", color);
    textureShader->setInt("textureSampler", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SpriteBatch::drawRect(float x, float y, float w, float h, const glm::vec4& color) {
    colorShader->use();
    colorShader->setMat4("projection", projection);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(w, h, 1.0f));
    
    colorShader->setMat4("model", model);
    colorShader->setVec4("color", color);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SpriteBatch::end() {

}

void SpriteBatch::setProjection(float width, float height) {
    projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
}