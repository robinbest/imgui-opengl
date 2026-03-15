#ifndef opengl_shader_hpp
#define opengl_shader_hpp

#include <string>
#include <GL/glew.h>

class Shader {
public:
    Shader();
    void init(const std::string& vertex_code, const std::string& fragment_code);
    void use();
    void setUniformMat4(const std::string& name, const float* val);

    template <typename T> void setUniform(const std::string& name, T val);
    template <typename T> void setUniform(const std::string& name, T val1, T val2);
    template <typename T> void setUniform(const std::string& name, T val1, T val2, T val3);

private:
    void checkCompileErr(GLuint shader, const char* stage);
    void checkLinkingErr();

private:
    GLuint id_ = 0;
    GLuint vertex_shader_ = 0;
    GLuint fragment_shader_ = 0;
};

#endif
