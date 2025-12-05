#include "TextRenderer.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Shader for text rendering
namespace TextShaders {
    const char* VERTEX = R"(
        #version 330 core
        layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
        out vec2 TexCoords;

        uniform mat4 projection;

        void main() {
            gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }
    )";

    const char* FRAGMENT = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D text;
        uniform vec4 textColor;

        void main() {
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
            color = textColor * sampled;
        }
    )";
}

TextRenderer::TextRenderer() 
    : ft_(nullptr), face_(nullptr), shader_(nullptr), vao_(0), vbo_(0) {
}

TextRenderer::~TextRenderer() {
    // Clean up character textures
    for (auto& pair : characters_) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    
    if (face_) {
        FT_Done_Face(face_);
    }
    
    if (ft_) {
        FT_Done_FreeType(ft_);
    }
    
    if (shader_) {
        delete shader_;
    }
    
    if (vao_) {
        glDeleteVertexArrays(1, &vao_);
    }
    
    if (vbo_) {
        glDeleteBuffers(1, &vbo_);
    }
}

bool TextRenderer::init(const std::string& fontPath, unsigned int fontSize) {
    // Initialize FreeType
    if (FT_Init_FreeType(&ft_)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    if (!loadFont(fontPath, fontSize)) {
        return false;
    }

    // Create shader
    shader_ = new Shader(TextShaders::VERTEX, TextShaders::FRAGMENT);

    // Set up VAO/VBO for rendering quads
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool TextRenderer::loadFont(const std::string& fontPath, unsigned int fontSize) {
    // Load font face
    if (FT_New_Face(ft_, fontPath.c_str(), 0, &face_)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font from " << fontPath << std::endl;
        return false;
    }

    // Set pixel size
    FT_Set_Pixel_Sizes(face_, 0, fontSize);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 ASCII characters
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face_, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph for char " << c << std::endl;
            continue;
        }

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face_->glyph->bitmap.width,
            face_->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face_->glyph->bitmap.buffer
        );

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Store character
        Character character = {
            texture,
            glm::ivec2(face_->glyph->bitmap.width, face_->glyph->bitmap.rows),
            glm::ivec2(face_->glyph->bitmap_left, face_->glyph->bitmap_top),
            static_cast<unsigned int>(face_->glyph->advance.x)
        };
        
        characters_.insert(std::pair<char, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

void TextRenderer::renderText(const std::string& text, float x, float y, float scale, 
                               const glm::vec4& color, float screenWidth, float screenHeight) {
    // Set up projection
    glm::mat4 projection = glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);
    
    // Activate shader and set uniforms
    shader_->use();
    shader_->setMat4("projection", projection);
    shader_->setVec4("textColor", color);
    shader_->setInt("text", 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao_);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Iterate through all characters
    float currentX = x;
    for (char c : text) {
        if (c == '\n') {
            // Handle newlines
            currentX = x;
            y += (face_->size->metrics.height >> 6) * scale;
            continue;
        }

        auto it = characters_.find(c);
        if (it == characters_.end()) {
            continue; // Character not loaded
        }

        Character ch = it->second;

        float xpos = currentX + ch.bearing.x * scale;
        float ypos = y + (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f },

            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f }
        };

        // Render glyph texture
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        
        // Update VBO
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Advance cursor (note: advance is 1/64th pixels)
        currentX += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

glm::vec2 TextRenderer::measureText(const std::string& text, float scale) {
    float width = 0.0f;
    float maxHeight = 0.0f;
    float lineWidth = 0.0f;
    int lineCount = 1;

    for (char c : text) {
        if (c == '\n') {
            if (lineWidth > width) {
                width = lineWidth;
            }
            lineWidth = 0.0f;
            lineCount++;
            continue;
        }

        auto it = characters_.find(c);
        if (it == characters_.end()) {
            continue;
        }

        Character ch = it->second;
        lineWidth += (ch.advance >> 6) * scale;
        
        float charHeight = ch.size.y * scale;
        if (charHeight > maxHeight) {
            maxHeight = charHeight;
        }
    }

    if (lineWidth > width) {
        width = lineWidth;
    }

    float totalHeight = lineCount * getLineHeight(scale);

    return glm::vec2(width, totalHeight);
}

float TextRenderer::getLineHeight(float scale) {
    if (face_) {
        return (face_->size->metrics.height >> 6) * scale;
    }
    return 0.0f;
}