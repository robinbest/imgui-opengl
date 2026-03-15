#include "opengl_shader.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

Shader::Shader() { }

void Shader::init(const std::string& vertex_code, const std::string& fragment_code) {
    vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_code_char = vertex_code.c_str();
    glShaderSource(vertex_shader_, 1, &vertex_code_char, NULL);
    glCompileShader(vertex_shader_);
    checkCompileErr(vertex_shader_, "VERTEX");

    fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_code_char = fragment_code.c_str();
    glShaderSource(fragment_shader_, 1, &fragment_code_char, NULL);
    glCompileShader(fragment_shader_);
    checkCompileErr(fragment_shader_, "FRAGMENT");

    id_ = glCreateProgram();
    glAttachShader(id_, vertex_shader_);
    glAttachShader(id_, fragment_shader_);
    glLinkProgram(id_);
    checkLinkingErr();

    glDeleteShader(vertex_shader_);
    glDeleteShader(fragment_shader_);
}

void Shader::use() {
    glUseProgram(id_);
}

void Shader::setUniformMat4(const std::string& name, const float* val) {
    glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, val);
}

template<> void Shader::setUniform(const std::string& name, int val) {
    glUniform1i(glGetUniformLocation(id_, name.c_str()), val);
}

template<> void Shader::setUniform(const std::string& name, float val) {
    glUniform1f(glGetUniformLocation(id_, name.c_str()), val);
}

template<> void Shader::setUniform(const std::string& name, float val1, float val2) {
    glUniform2f(glGetUniformLocation(id_, name.c_str()), val1, val2);
}

template<> void Shader::setUniform(const std::string& name, float val1, float val2, float val3) {
    glUniform3f(glGetUniformLocation(id_, name.c_str()), val1, val2, val3);
}

void Shader::checkCompileErr(GLuint shader, const char* stage) {
    int success = 0;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::" << stage << "::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }
}

void Shader::checkLinkingErr() {
    int success;
    char infoLog[1024];
    glGetProgramiv(id_, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(id_, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}
