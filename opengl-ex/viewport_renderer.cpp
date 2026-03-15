#include "viewport_renderer.h"

#include <iostream>

namespace
{
constexpr float kPi = 3.14159265358979323846f;
}

bool ViewportRenderer::initialize(Shader* shader)
{
    shader_ = shader;
    create_fbo(width_, height_);
    return true;
}

void ViewportRenderer::shutdown()
{
    if (fbo_ != 0) glDeleteFramebuffers(1, &fbo_);
    if (color_tex_ != 0) glDeleteTextures(1, &color_tex_);
    if (depth_rbo_ != 0) glDeleteRenderbuffers(1, &depth_rbo_);
    fbo_ = 0;
    color_tex_ = 0;
    depth_rbo_ = 0;
}

void ViewportRenderer::resize(int width, int height)
{
    width_ = (width < 1) ? 1 : width;
    height_ = (height < 1) ? 1 : height;
    create_fbo(width_, height_);
}

void ViewportRenderer::create_fbo(int width, int height)
{
    if (fbo_ != 0)
    {
        glDeleteFramebuffers(1, &fbo_);
        glDeleteTextures(1, &color_tex_);
        glDeleteRenderbuffers(1, &depth_rbo_);
        fbo_ = 0;
        color_tex_ = 0;
        depth_rbo_ = 0;
    }

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glGenTextures(1, &color_tex_);
    glBindTexture(GL_TEXTURE_2D, color_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex_, 0);

    glGenRenderbuffers(1, &depth_rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_rbo_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR: viewport framebuffer is not complete.\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewportRenderer::render(const AppState& app, const CubeMesh& cube)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, width_, height_);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.12f, 0.13f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
    const Mat4 proj = perspective(45.0f * kPi / 180.0f, aspect, 0.1f, 100.0f);
    const Mat4 model = identity();
    const Mat4 view = app.camera.view_matrix();
    const Mat4 mvp = multiply(proj, multiply(view, model));

    shader_->use();
    shader_->setUniformMat4("u_mvp", mvp.v);

    cube.bind();
    for (int face = 0; face < 6; ++face)
    {
        if (face == app.picked_face)
            shader_->setUniform("tint", 1.0f, 1.0f, 0.2f);
        else
            shader_->setUniform("tint", app.tint[0], app.tint[1], app.tint[2]);

        cube.draw_face(face);
    }
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
