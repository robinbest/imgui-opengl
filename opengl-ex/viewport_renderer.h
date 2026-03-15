#pragma once

#include "app_state.h"
#include "cube_mesh.h"
#include "math3d.h"
#include "opengl_shader.h"

#include <GL/glew.h>

class ViewportRenderer
{
public:
    bool initialize(Shader* shader);
    void shutdown();

    void resize(int width, int height);
    void render(const AppState& app, const CubeMesh& cube);
    GLuint texture_id() const { return color_tex_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    void create_fbo(int width, int height);

    Shader* shader_ = nullptr;
    GLuint fbo_ = 0;
    GLuint color_tex_ = 0;
    GLuint depth_rbo_ = 0;
    int width_ = 1;
    int height_ = 1;
};
