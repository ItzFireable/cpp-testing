#pragma once
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

namespace Shaders {
    const char* TEXTURE_VERTEX = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        uniform mat4 projection;
        uniform mat4 model;
        
        void main() {
            gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char* TEXTURE_FRAGMENT = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        
        uniform sampler2D textureSampler;
        uniform vec4 color;
        
        void main() {
            FragColor = texture(textureSampler, TexCoord) * color;
        }
    )";

    const char* COLOR_VERTEX = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        
        uniform mat4 projection;
        uniform mat4 model;
        
        void main() {
            gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* COLOR_FRAGMENT = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec4 color;
        
        void main() {
            FragColor = color;
        }
    )";
}

class Shader {
public:
    GLuint program;

    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    void use();
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat4(const std::string& name, const glm::mat4& mat);

private:
    GLuint compileShader(const char* source, GLenum type);
};